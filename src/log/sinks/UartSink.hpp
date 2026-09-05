#ifndef AM335X_LOG_UART_SINK_HPP
#define AM335X_LOG_UART_SINK_HPP

#include "log/log.h"
#include <cstddef>
#include <cstdint>

namespace HAL::LOG
{
    // Non-owning, synchronous polling transport. UartT must outlive this object.
    // Use from one serialized foreground context; not ISR/panic/task-safe.
    // Does not initialize hardware, switch modes, allocate DMA, or allocate heap.
    template<typename UartT>
    class UartSink final
    {
        UartT& m_uart;
        log_sink_t m_sink;
        bool m_registered;
        bool m_enabled{true};
        bool m_writing{false};
        uint32_t m_timeout_loops;
        uint32_t m_delivered{0u};
        uint32_t m_dropped{0u};

        static void dispatch(void* context, log_level_t,
                             const char* data, std::size_t size)
        {
            static_cast<UartSink*>(context)->write(data, size);
        }

        void write(const char* data, const std::size_t size) noexcept
        {
#if defined(__arm__)
            // Existing BSP abort/IRQ diagnostics still reach the common logger.
            // Never start polling UART output from those exception modes.
            uint32_t cpsr;
            __asm volatile("mrs %0, cpsr" : "=r"(cpsr));
            const uint32_t mode = cpsr & 0x1fu;
            if (mode == 0x11u || mode == 0x12u || mode == 0x17u || mode == 0x1bu)
                return; // Do not mutate foreground counters from exceptions.
#endif
            using Mode = decltype(m_uart.io_mode());
            // Checking software state is safe before UART clocks are enabled.
            if (!m_enabled || m_writing || m_uart.io_mode() != Mode::POLLING)
            {
                ++m_dropped;
                return;
            }
            m_writing = true;
            bool ok = true;
            // Translate bare LF to CRLF. Existing CRLF is not doubled.
            std::size_t start = 0u;
            for (std::size_t i = 0u; i < size && ok; ++i)
            {
                if (data[i] == '\n' && (i == 0u || data[i - 1u] != '\r'))
                {
                    if (i != start)
                        ok = m_uart.put_data_bounded(data + start, i - start,
                                                     m_timeout_loops);
                    if (ok)
                        ok = m_uart.put_data_bounded("\r\n", 2u, m_timeout_loops);
                    start = i + 1u;
                }
            }
            if (ok && start < size)
                ok = m_uart.put_data_bounded(data + start, size - start,
                                             m_timeout_loops);
            if (ok)
                ++m_delivered;
            else
                ++m_dropped; // A timed-out record may have been partially sent.
            m_writing = false;
        }

    public:
        explicit UartSink(UartT& uart, uint32_t timeout_loops = 5'000'000u) noexcept
            : m_uart(uart), m_sink{this, dispatch},
              m_registered(log_register_sink(&m_sink) != 0),
              m_timeout_loops(timeout_loops)
        { }

        ~UartSink() noexcept
        {
            if (m_registered)
                log_unregister_sink(&m_sink);
        }
        UartSink(const UartSink&) = delete;
        UartSink& operator=(const UartSink&) = delete;
        UartSink(UartSink&&) = delete;
        UartSink& operator=(UartSink&&) = delete;

        [[nodiscard]] bool registered() const noexcept { return m_registered; }
        void enable(bool enabled = true) noexcept { m_enabled = enabled; }
        [[nodiscard]] bool enabled() const noexcept { return m_enabled; }
        [[nodiscard]] uint32_t delivered() const noexcept { return m_delivered; }
        [[nodiscard]] uint32_t dropped() const noexcept { return m_dropped; }
    };
}

#endif
