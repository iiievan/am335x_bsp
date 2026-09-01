#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "hal/PERF.hpp"
#include "rtt/rtt_log.h"
#include "uart_tests.hpp"

#define TAG "uart_cli"

namespace
{
    using Uart = HAL::UART::uart0_t;

    constexpr uint32_t TEST_TIMEOUT_LOOPS = 500'000'000u;
    constexpr uint32_t RECOVERY_TIMEOUT_LOOPS = 1'000'000u;

    constexpr uint32_t DMA_FRAME_MAGIC = 0x55415254u; // "TRAU" in LE memory
    constexpr uint16_t DMA_FRAME_VERSION = 1u;
    constexpr std::size_t DMA_FRAME_HEADER_SIZE = 16u;
    constexpr std::size_t DMA_FRAME_CRC_SIZE = 2u;
    constexpr std::size_t DMA_ALIGNMENT = 8u;
    constexpr std::size_t DMA_MAX_PAYLOAD_SIZE = 6144u;
    constexpr std::size_t DMA_MAX_FRAME_SIZE =
        DMA_FRAME_HEADER_SIZE + DMA_MAX_PAYLOAD_SIZE + 6u + DMA_FRAME_CRC_SIZE;
    constexpr uint32_t UART_BAUD_VALUES[] =
    {
        300u, 600u, 1200u, 2400u, 4800u, 9600u, 14400u, 19200u, 28800u,
        38400u, 57600u, 115200u, 230400u, 460800u, 921600u, 1843200u,
        3686400u
    };
    constexpr std::size_t UART_BAUD_COUNT =
        sizeof(UART_BAUD_VALUES) / sizeof(UART_BAUD_VALUES[0]);

    enum class AutoMode : uint8_t
    {
        POLLING,
        ISR,
        DMA
    };

    uint8_t* volatile g_auto_interrupt_buffer{nullptr};
    volatile std::size_t g_auto_interrupt_expected{0u};
    volatile std::size_t g_auto_interrupt_count{0u};
    volatile bool g_auto_interrupt_active{false};
    volatile bool g_auto_interrupt_complete{false};
    alignas(64) uint8_t g_dma_frame[DMA_MAX_FRAME_SIZE]{};
    constexpr std::size_t GUARD_SIZE = 64u;
    constexpr uint8_t GUARD_VALUE = 0xA5u;
    alignas(64) uint8_t g_unaligned_storage[
        GUARD_SIZE + DMA_MAX_FRAME_SIZE + GUARD_SIZE + 32u]{};

    void log_dma_hw_state(const char* phase, const uint8_t channel) noexcept
    {
        using namespace REGS::EDMA;
        const auto region = HAL::EDMA::get_region_id();
        const auto& cc = *AM335X_EDMA3CC;
        const auto& uart = *REGS::UART::AM335X_UART_0;
        const auto* words = reinterpret_cast<const volatile uint32_t*>(&cc.paRAM(channel));

        RTT_LOG_I("dma_auto",
                  "%s ch=%u UART LSR=%08x SSR=%08x SCR=%08x RXFIFO=%u TXFIFO=%u",
                  phase, static_cast<unsigned>(channel),
                  static_cast<unsigned>(uart.LSR_UART.reg),
                  static_cast<unsigned>(uart.SSR.reg),
                  static_cast<unsigned>(uart.SCR.reg),
                  static_cast<unsigned>(uart.RXFIFO_LVL.b.RXFIFO_LVL),
                  static_cast<unsigned>(uart.TXFIFO_LVL.b.TXFIFO_LVL));
        RTT_LOG_I("dma_auto",
                  "%s ER=%08x EER=%08x SER=%08x IPR=%08x EMR=%08x CCERR=%08x DRAE=%08x",
                  phase,
                  static_cast<unsigned>(cc.S_ER(region).reg),
                  static_cast<unsigned>(cc.S_EER(region).reg),
                  static_cast<unsigned>(cc.S_SER(region).reg),
                  static_cast<unsigned>(cc.S_IPR(region).reg),
                  static_cast<unsigned>(cc.EMR.reg),
                  static_cast<unsigned>(cc.CCERR.reg),
                  static_cast<unsigned>(cc.DRAE(region).reg));
        RTT_LOG_I("dma_auto", "%s PaRAM[%u] %08x %08x %08x %08x",
                  phase, static_cast<unsigned>(channel),
                  static_cast<unsigned>(words[0]), static_cast<unsigned>(words[1]),
                  static_cast<unsigned>(words[2]), static_cast<unsigned>(words[3]));
        RTT_LOG_I("dma_auto", "%s PaRAM[%u] %08x %08x %08x %08x",
                  phase, static_cast<unsigned>(channel),
                  static_cast<unsigned>(words[4]), static_cast<unsigned>(words[5]),
                  static_cast<unsigned>(words[6]), static_cast<unsigned>(words[7]));
    }

    class PollingRestore final
    {
    public:
        explicit PollingRestore(Uart& uart) noexcept : m_uart(uart) {}
        ~PollingRestore() noexcept { m_uart.init_polling(); }

        PollingRestore(const PollingRestore&) = delete;
        PollingRestore& operator=(const PollingRestore&) = delete;

    private:
        Uart& m_uart;
    };

    struct DmaFrameHeader
    {
        uint32_t magic;
        uint16_t version;
        uint16_t sequence;
        uint32_t payload_size;
        uint32_t seed;
    };
    static_assert(sizeof(DmaFrameHeader) == DMA_FRAME_HEADER_SIZE);

    [[nodiscard]] constexpr std::size_t dma_frame_size(const std::size_t payload_size) noexcept
    {
        const std::size_t without_padding =
            DMA_FRAME_HEADER_SIZE + payload_size + DMA_FRAME_CRC_SIZE;
        return (without_padding + DMA_ALIGNMENT - 1u) & ~(DMA_ALIGNMENT - 1u);
    }

    [[nodiscard]] uint16_t crc16_ccitt_false(const uint8_t* data,
                                             const std::size_t size) noexcept
    {
        uint16_t crc = 0xFFFFu;
        for (std::size_t i = 0u; i < size; ++i)
        {
            crc ^= static_cast<uint16_t>(data[i]) << 8u;
            for (uint8_t bit = 0u; bit < 8u; ++bit)
                crc = (crc & 0x8000u) != 0u
                          ? static_cast<uint16_t>((crc << 1u) ^ 0x1021u)
                          : static_cast<uint16_t>(crc << 1u);
        }
        return crc;
    }

    [[nodiscard]] uint32_t xorshift32(uint32_t& state) noexcept
    {
        state ^= state << 13u;
        state ^= state >> 17u;
        state ^= state << 5u;
        return state;
    }

    [[nodiscard]] bool parse_u32(const char*& input, uint32_t& value) noexcept
    {
        while (*input == ' ')
            ++input;
        if (*input < '0' || *input > '9')
            return false;

        uint32_t result = 0u;
        do
        {
            const uint32_t digit = static_cast<uint32_t>(*input - '0');
            if (result > (0xFFFFFFFFu - digit) / 10u)
                return false;
            result = result * 10u + digit;
            ++input;
        } while (*input >= '0' && *input <= '9');

        value = result;
        return true;
    }

    [[nodiscard]] bool parse_auto_mode(const char*& input, AutoMode& mode) noexcept
    {
        while (*input == ' ')
            ++input;

        struct ModeName
        {
            const char* name;
            std::size_t length;
            AutoMode mode;
        };
        constexpr ModeName modes[] =
        {
            {"polling", 7u, AutoMode::POLLING},
            {"isr", 3u, AutoMode::ISR},
            {"dma", 3u, AutoMode::DMA}
        };

        for (const auto& candidate : modes)
        {
            std::size_t index = 0u;
            while (index < candidate.length && input[index] == candidate.name[index])
                ++index;
            if (index == candidate.length &&
                (input[index] == ' ' || input[index] == '\0'))
            {
                input += index;
                mode = candidate.mode;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool parse_auto_dma(const char* command,
                                      AutoMode& mode,
                                      uint32_t& payload_size,
                                      uint32_t& sequence,
                                      uint32_t& seed) noexcept
    {
        constexpr char prefix[] = "auto loop";
        for (std::size_t i = 0u; i < sizeof(prefix) - 1u; ++i)
        {
            if (command[i] != prefix[i])
                return false;
        }

        const char* cursor = command + sizeof(prefix) - 1u;
        if (!parse_auto_mode(cursor, mode) ||
            !parse_u32(cursor, payload_size) ||
            !parse_u32(cursor, sequence) ||
            !parse_u32(cursor, seed))
        {
            return false;
        }
        while (*cursor == ' ')
            ++cursor;
        return *cursor == '\0';
    }

    [[nodiscard]] bool parse_auto_tx(const char* command,
                                     AutoMode& mode,
                                     uint32_t& transfer_size,
                                     uint32_t& sequence,
                                     uint32_t& seed) noexcept
    {
        constexpr char prefix[] = "auto tx";
        for (std::size_t i = 0u; i < sizeof(prefix) - 1u; ++i)
        {
            if (command[i] != prefix[i])
                return false;
        }

        const char* cursor = command + sizeof(prefix) - 1u;
        if (!parse_auto_mode(cursor, mode) ||
            !parse_u32(cursor, transfer_size) ||
            !parse_u32(cursor, sequence) ||
            !parse_u32(cursor, seed))
        {
            return false;
        }
        while (*cursor == ' ')
            ++cursor;
        return *cursor == '\0';
    }

    [[nodiscard]] bool parse_auto_rx(const char* command,
                                     AutoMode& mode,
                                     uint32_t& transfer_size,
                                     uint32_t& sequence,
                                     uint32_t& seed) noexcept
    {
        constexpr char prefix[] = "auto rx";
        for (std::size_t i = 0u; i < sizeof(prefix) - 1u; ++i)
        {
            if (command[i] != prefix[i])
                return false;
        }

        const char* cursor = command + sizeof(prefix) - 1u;
        if (!parse_auto_mode(cursor, mode) ||
            !parse_u32(cursor, transfer_size) ||
            !parse_u32(cursor, sequence) ||
            !parse_u32(cursor, seed))
        {
            return false;
        }
        while (*cursor == ' ')
            ++cursor;
        return *cursor == '\0';
    }

    [[nodiscard]] bool parse_auto_special(const char* command,
                                          const char* prefix,
                                          const std::size_t prefix_length,
                                          AutoMode& mode,
                                          uint32_t& payload_size,
                                          uint32_t& sequence,
                                          uint32_t& seed) noexcept
    {
        for (std::size_t i = 0u; i < prefix_length; ++i)
            if (command[i] != prefix[i])
                return false;
        const char* cursor = command + prefix_length;
        if (!parse_auto_mode(cursor, mode) ||
            !parse_u32(cursor, payload_size) ||
            !parse_u32(cursor, sequence) ||
            !parse_u32(cursor, seed))
        {
            return false;
        }
        while (*cursor == ' ')
            ++cursor;
        return *cursor == '\0';
    }

    [[nodiscard]] bool parse_auto_recover(const char* command,
                                          AutoMode& mode,
                                          uint32_t& payload_size,
                                          uint32_t& sequence,
                                          uint32_t& seed) noexcept
    {
        constexpr char prefix[] = "auto recover";
        return parse_auto_special(command, prefix, sizeof(prefix) - 1u,
                                  mode, payload_size, sequence, seed);
    }

    [[nodiscard]] bool parse_auto_offset(const char* command,
                                         AutoMode& mode,
                                         uint32_t& payload_size,
                                         uint32_t& sequence,
                                         uint32_t& seed) noexcept
    {
        constexpr char prefix[] = "auto offset";
        return parse_auto_special(command, prefix, sizeof(prefix) - 1u,
                                  mode, payload_size, sequence, seed);
    }

    [[nodiscard]] bool parse_auto_baud(const char* command,
                                       uint32_t& baud_index) noexcept
    {
        constexpr char prefix[] = "auto baud";
        for (std::size_t i = 0u; i < sizeof(prefix) - 1u; ++i)
        {
            if (command[i] != prefix[i])
                return false;
        }

        const char* cursor = command + sizeof(prefix) - 1u;
        if (!parse_u32(cursor, baud_index))
            return false;
        while (*cursor == ' ')
            ++cursor;
        return *cursor == '\0';
    }

    [[nodiscard]] uint32_t baud_value(const Uart& uart) noexcept
    {
        const auto index = static_cast<uint32_t>(uart.baudrate());
        return index < UART_BAUD_COUNT ? UART_BAUD_VALUES[index] : 115200u;
    }

    [[nodiscard]] uint32_t dma_timeout_epochs(const Uart& uart) noexcept
    {
        const uint32_t baud = baud_value(uart);
        const uint32_t ratio = (115200u + baud - 1u) / baud;
        return ratio > 1u ? ratio * 2u : 1u;
    }

    [[nodiscard]] const char* auto_mode_name(const AutoMode mode) noexcept
    {
        switch (mode)
        {
            case AutoMode::POLLING: return "polling";
            case AutoMode::ISR:     return "isr";
            case AutoMode::DMA:     return "dma";
        }
        return "unknown";
    }

    void auto_interrupt_rx_callback(const char value)
    {
        if (!g_auto_interrupt_active || g_auto_interrupt_buffer == nullptr)
            return;

        const std::size_t index = g_auto_interrupt_count;
        if (index < g_auto_interrupt_expected)
        {
            g_auto_interrupt_buffer[index] = static_cast<uint8_t>(value);
            g_auto_interrupt_count = index + 1u;
        }
        if (g_auto_interrupt_count == g_auto_interrupt_expected)
        {
            g_auto_interrupt_active = false;
            g_auto_interrupt_complete = true;
        }
    }

    void reset_auto_interrupt_receiver() noexcept
    {
        g_auto_interrupt_active = false;
        g_auto_interrupt_complete = false;
        g_auto_interrupt_count = 0u;
        g_auto_interrupt_expected = 0u;
        g_auto_interrupt_buffer = nullptr;
    }

    [[nodiscard]] bool wait_for_rx_byte(Uart& uart, uint8_t& value,
                                        const uint32_t timeout_epochs) noexcept
    {
        for (uint32_t epoch = 0u; epoch < timeout_epochs; ++epoch)
        {
            uint32_t remaining = TEST_TIMEOUT_LOOPS;
            while (!uart.rx_data_available() && remaining != 0u)
            {
                --remaining;
                __asm volatile("nop");
            }
            if (uart.rx_data_available())
            {
                value = static_cast<uint8_t>(uart.get_char());
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool receive_polling(Uart& uart, uint8_t* data,
                                       const std::size_t size) noexcept
    {
        const uint32_t timeout_epochs = dma_timeout_epochs(uart);
        for (std::size_t index = 0u; index < size; ++index)
        {
            if (!wait_for_rx_byte(uart, data[index], timeout_epochs))
                return false;
        }
        return true;
    }

    [[nodiscard]] bool start_interrupt_receive(Uart& uart, uint8_t* data,
                                               const std::size_t size) noexcept
    {
        reset_auto_interrupt_receiver();
        g_auto_interrupt_buffer = data;
        g_auto_interrupt_expected = size;
        g_auto_interrupt_active = true;
        if (!uart.init_interrupt(auto_interrupt_rx_callback))
        {
            reset_auto_interrupt_receiver();
            return false;
        }

        return true;
    }

    [[nodiscard]] bool wait_interrupt_receive(Uart& uart,
                                              const std::size_t size) noexcept
    {
        const uint32_t timeout_epochs = dma_timeout_epochs(uart);
        for (uint32_t epoch = 0u;
             epoch < timeout_epochs && !g_auto_interrupt_complete;
             ++epoch)
        {
            uint32_t remaining = TEST_TIMEOUT_LOOPS;
            while (!g_auto_interrupt_complete && remaining != 0u)
            {
                --remaining;
                __asm volatile("nop");
            }
        }

        const bool complete = g_auto_interrupt_complete &&
                              g_auto_interrupt_count == size;
        reset_auto_interrupt_receiver();
        return complete;
    }

    void run_baud_switch(Uart& uart, const uint32_t baud_index) noexcept
    {
        const auto old_baud = uart.baudrate();
        const uint32_t old_index = static_cast<uint32_t>(old_baud);
        const auto new_baud = static_cast<REGS::UART::e_BAUDRATE>(baud_index);
        char message[160]{};

        std::snprintf(message, sizeof(message),
                      "@BAUD READY old_index=%u old=%u new_index=%u new=%u\n",
                      static_cast<unsigned>(old_index),
                      static_cast<unsigned>(UART_BAUD_VALUES[old_index]),
                      static_cast<unsigned>(baud_index),
                      static_cast<unsigned>(UART_BAUD_VALUES[baud_index]));
        uart.put_string(message);
        uart.wait_tx_complete();

        if (!uart.set_baudrate(new_baud))
        {
            (void)uart.set_baudrate(old_baud);
            uart.put_string("@BAUD ROLLBACK error=SET_BAUD\n");
            return;
        }

        const uint8_t expected_sync[] =
        {
            0x55u, 0xAAu, static_cast<uint8_t>(baud_index),
            static_cast<uint8_t>(baud_index ^ 0xFFu)
        };
        bool sync_ok = true;
        for (const uint8_t expected : expected_sync)
        {
            uint32_t timeout = TEST_TIMEOUT_LOOPS;
            while (!uart.rx_data_available() && timeout != 0u)
            {
                --timeout;
                __asm volatile("nop");
            }
            if (timeout == 0u || static_cast<uint8_t>(uart.get_char()) != expected)
            {
                sync_ok = false;
                break;
            }
        }

        if (!sync_ok)
        {
            uart.FIFO_clear(false, true);
            (void)uart.set_baudrate(old_baud);
            std::snprintf(message, sizeof(message),
                          "@BAUD ROLLBACK index=%u baud=%u error=SYNC\n",
                          static_cast<unsigned>(old_index),
                          static_cast<unsigned>(UART_BAUD_VALUES[old_index]));
            uart.put_string(message);
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@BAUD ACTIVE index=%u baud=%u mode=%ux\n",
                      static_cast<unsigned>(baud_index),
                      static_cast<unsigned>(UART_BAUD_VALUES[baud_index]),
                      baud_index >= static_cast<uint32_t>(REGS::UART::KBPS_480_8)
                          ? 13u : 16u);
        uart.put_string(message);
        RTT_LOG_I("dma_auto", "BAUD old=%u new=%u index=%u mode=%ux",
                  static_cast<unsigned>(UART_BAUD_VALUES[old_index]),
                  static_cast<unsigned>(UART_BAUD_VALUES[baud_index]),
                  static_cast<unsigned>(baud_index),
                  baud_index >= static_cast<uint32_t>(REGS::UART::KBPS_480_8)
                      ? 13u : 16u);
    }

    void build_tx_test_packet(const std::size_t size, const uint32_t seed) noexcept
    {
        uint32_t state = seed != 0u ? seed : 0x6D2B79F5u;
        const std::size_t data_size = size - DMA_FRAME_CRC_SIZE;
        for (std::size_t i = 0u; i < data_size; ++i)
            g_dma_frame[i] = static_cast<uint8_t>(xorshift32(state) & 0xFFu);

        const uint16_t crc = crc16_ccitt_false(g_dma_frame, data_size);
        g_dma_frame[data_size] = static_cast<uint8_t>(crc & 0xFFu);
        g_dma_frame[data_size + 1u] = static_cast<uint8_t>(crc >> 8u);
    }

    void run_tx_tail_test(Uart& uart,
                          const uint32_t transfer_size,
                          const uint16_t sequence,
                          const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        char message[192]{};
        build_tx_test_packet(transfer_size, seed);

        RTT_LOG_I("dma_auto", "TXTAIL BEGIN seq=%u size=%u dma=%u tail=%u seed=%08x",
                  static_cast<unsigned>(sequence),
                  static_cast<unsigned>(transfer_size),
                  static_cast<unsigned>(transfer_size - transfer_size % DMA_ALIGNMENT),
                  static_cast<unsigned>(transfer_size % DMA_ALIGNMENT),
                  static_cast<unsigned>(seed));

        if (!uart.init_dma())
        {
            RTT_LOG_E("dma_auto", "TXTAIL UART DMA initialization failed");
            uart.init_polling();
            uart.put_string("@RESULT mode=tx status=FAIL error=DMA_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=tx sequence=%u size=%u dma=%u tail=%u\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size),
                      static_cast<unsigned>(transfer_size - transfer_size % DMA_ALIGNMENT),
                      static_cast<unsigned>(transfer_size % DMA_ALIGNMENT));
        uart.put_string(message);
        uart.wait_tx_complete();

        const uint32_t tx_started = HAL::PERF::get_cycle_count();
        const bool tx_ok = uart.write(g_dma_frame, transfer_size,
                                      TEST_TIMEOUT_LOOPS,
                                      dma_timeout_epochs(uart));
        const uint32_t tx_cycles = HAL::PERF::get_cycle_count() - tx_started;
        if (!tx_ok)
            log_dma_hw_state("TXTAIL_FAIL", REGS::EDMA::CH_UART0_TX);

        uart.init_polling();
        std::snprintf(message, sizeof(message),
                      "@RESULT mode=tx sequence=%u size=%u tail=%u tx=%u status=%s\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size),
                      static_cast<unsigned>(transfer_size % DMA_ALIGNMENT),
                      tx_ok ? static_cast<unsigned>(transfer_size) : 0u,
                      tx_ok ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("dma_auto", "TXTAIL END seq=%u status=%s cycles=%u",
                  static_cast<unsigned>(sequence), tx_ok ? "PASS" : "FAIL",
                  static_cast<unsigned>(tx_cycles));
    }

    void run_rx_tail_test(Uart& uart,
                          const uint32_t transfer_size,
                          const uint16_t sequence,
                          const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        char message[224]{};
        const std::size_t dma_size = transfer_size - transfer_size % DMA_ALIGNMENT;
        const std::size_t tail_size = transfer_size % DMA_ALIGNMENT;

        RTT_LOG_I("dma_auto", "RXTAIL BEGIN seq=%u size=%u dma=%u tail=%u seed=%08x",
                  static_cast<unsigned>(sequence),
                  static_cast<unsigned>(transfer_size),
                  static_cast<unsigned>(dma_size),
                  static_cast<unsigned>(tail_size),
                  static_cast<unsigned>(seed));

        if (!uart.init_dma())
        {
            RTT_LOG_E("dma_auto", "RXTAIL UART DMA initialization failed");
            uart.init_polling();
            uart.put_string("@RESULT mode=rx status=FAIL error=DMA_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=rx sequence=%u size=%u dma=%u tail=%u\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size),
                      static_cast<unsigned>(dma_size),
                      static_cast<unsigned>(tail_size));
        uart.put_string(message);
        uart.wait_tx_complete();

        RTT_LOG_I("dma_auto", "RXTAIL READY sent; entering blocking RX");
        const uint32_t rx_started = HAL::PERF::get_cycle_count();
        const bool rx_ok = uart.read(g_dma_frame, transfer_size,
                                     TEST_TIMEOUT_LOOPS,
                                     TEST_TIMEOUT_LOOPS,
                                     dma_timeout_epochs(uart));
        const bool hardware_timeout_ok = tail_size == 0u ||
                                         uart.last_rx_hardware_timeout();
        const uint32_t rx_cycles = HAL::PERF::get_cycle_count() - rx_started;

        bool crc_ok = false;
        bool data_ok = false;
        uint16_t received_crc = 0u;
        uint16_t calculated_crc = 0u;
        if (rx_ok)
        {
            received_crc =
                static_cast<uint16_t>(g_dma_frame[transfer_size - 2u]) |
                static_cast<uint16_t>(static_cast<uint16_t>(g_dma_frame[transfer_size - 1u]) << 8u);
            calculated_crc = crc16_ccitt_false(g_dma_frame,
                                               transfer_size - DMA_FRAME_CRC_SIZE);
            crc_ok = received_crc == calculated_crc;

            uint32_t state = seed != 0u ? seed : 0x6D2B79F5u;
            data_ok = true;
            for (std::size_t i = 0u; i < transfer_size - DMA_FRAME_CRC_SIZE; ++i)
            {
                if (g_dma_frame[i] != static_cast<uint8_t>(xorshift32(state) & 0xFFu))
                {
                    data_ok = false;
                    break;
                }
            }
        }

        RTT_LOG_I("dma_auto",
                  "RXTAIL %s after %u PMU cycles; crc=%s data=%s received_crc=%04x calculated_crc=%04x",
                  rx_ok ? "completed" : "failed",
                  static_cast<unsigned>(rx_cycles),
                  crc_ok ? "PASS" : "FAIL",
                  data_ok ? "PASS" : "FAIL",
                  static_cast<unsigned>(received_crc),
                  static_cast<unsigned>(calculated_crc));
        log_dma_hw_state(rx_ok ? "RXTAIL_DONE" : "RXTAIL_FAIL",
                         REGS::EDMA::CH_UART0_RX);

        uart.init_polling();
        const bool passed = rx_ok && crc_ok && data_ok && hardware_timeout_ok;
        std::snprintf(message, sizeof(message),
                      "@RESULT mode=rx sequence=%u size=%u dma=%u tail=%u rx=%u crc=%s data=%s hw_timeout=%s status=%s\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size),
                      static_cast<unsigned>(dma_size),
                      static_cast<unsigned>(tail_size),
                      rx_ok ? static_cast<unsigned>(transfer_size) : 0u,
                      crc_ok ? "PASS" : "FAIL",
                      data_ok ? "PASS" : "FAIL",
                      tail_size == 0u ? "N/A" :
                          (hardware_timeout_ok ? "PASS" : "FAIL"),
                      passed ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("dma_auto", "RXTAIL END seq=%u status=%s",
                  static_cast<unsigned>(sequence), passed ? "PASS" : "FAIL");
    }

    [[nodiscard]] bool validate_dma_payload(const DmaFrameHeader& header) noexcept
    {
        uint32_t state = header.seed != 0u ? header.seed : 0x6D2B79F5u;
        const auto* payload = g_dma_frame + DMA_FRAME_HEADER_SIZE;
        for (std::size_t i = 0u; i < header.payload_size; ++i)
        {
            if (payload[i] != static_cast<uint8_t>(xorshift32(state) & 0xFFu))
                return false;
        }
        return true;
    }

    [[nodiscard]] bool prepare_cpu_receive(Uart& uart, const AutoMode mode,
                                           uint8_t* data,
                                           const std::size_t size) noexcept
    {
        if (mode == AutoMode::POLLING)
        {
            uart.init_polling();
            return true;
        }
        return mode == AutoMode::ISR &&
               start_interrupt_receive(uart, data, size);
    }

    [[nodiscard]] bool finish_cpu_receive(Uart& uart, const AutoMode mode,
                                          uint8_t* data,
                                          const std::size_t size) noexcept
    {
        if (mode == AutoMode::POLLING)
            return receive_polling(uart, data, size);
        return mode == AutoMode::ISR && wait_interrupt_receive(uart, size);
    }

    void run_cpu_tx_test(Uart& uart, const AutoMode mode,
                         const uint32_t transfer_size,
                         const uint16_t sequence,
                         const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        char message[192]{};
        build_tx_test_packet(transfer_size, seed);
        reset_auto_interrupt_receiver();
        const bool initialized = mode == AutoMode::POLLING
            ? (uart.init_polling(), true)
            : uart.init_interrupt(auto_interrupt_rx_callback);
        if (!initialized)
        {
            uart.init_polling();
            uart.put_string("@RESULT mode=tx status=FAIL error=UART_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=tx transport=%s sequence=%u size=%u\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size));
        uart.put_string(message);
        uart.wait_tx_complete();
        const uint32_t started = HAL::PERF::get_cycle_count();
        uart.put_data(g_dma_frame, transfer_size);
        uart.wait_tx_complete();
        const uint32_t cycles = HAL::PERF::get_cycle_count() - started;
        uart.init_polling();
        std::snprintf(message, sizeof(message),
                      "@RESULT mode=tx transport=%s sequence=%u size=%u tx=%u status=PASS\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size),
                      static_cast<unsigned>(transfer_size));
        uart.put_string(message);
        RTT_LOG_I("uart_auto", "TX mode=%s seq=%u size=%u cycles=%u status=PASS",
                  auto_mode_name(mode), static_cast<unsigned>(sequence),
                  static_cast<unsigned>(transfer_size),
                  static_cast<unsigned>(cycles));
    }

    void run_cpu_rx_test(Uart& uart, const AutoMode mode,
                         const uint32_t transfer_size,
                         const uint16_t sequence,
                         const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        char message[224]{};
        if (!prepare_cpu_receive(uart, mode, g_dma_frame, transfer_size))
        {
            uart.init_polling();
            uart.put_string("@RESULT mode=rx status=FAIL error=UART_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=rx transport=%s sequence=%u size=%u\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size));
        uart.put_string(message);
        uart.wait_tx_complete();
        const uint32_t started = HAL::PERF::get_cycle_count();
        const bool rx_ok = finish_cpu_receive(uart, mode, g_dma_frame,
                                              transfer_size);
        const uint32_t cycles = HAL::PERF::get_cycle_count() - started;

        bool crc_ok = false;
        bool data_ok = false;
        if (rx_ok)
        {
            const uint16_t received_crc =
                static_cast<uint16_t>(g_dma_frame[transfer_size - 2u]) |
                static_cast<uint16_t>(static_cast<uint16_t>(g_dma_frame[transfer_size - 1u]) << 8u);
            crc_ok = received_crc == crc16_ccitt_false(
                g_dma_frame, transfer_size - DMA_FRAME_CRC_SIZE);
            uint32_t state = seed != 0u ? seed : 0x6D2B79F5u;
            data_ok = true;
            for (std::size_t i = 0u;
                 i < transfer_size - DMA_FRAME_CRC_SIZE; ++i)
            {
                if (g_dma_frame[i] !=
                    static_cast<uint8_t>(xorshift32(state) & 0xFFu))
                {
                    data_ok = false;
                    break;
                }
            }
        }

        uart.init_polling();
        const bool passed = rx_ok && crc_ok && data_ok;
        std::snprintf(message, sizeof(message),
                      "@RESULT mode=rx transport=%s sequence=%u size=%u rx=%u crc=%s data=%s status=%s\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      static_cast<unsigned>(transfer_size),
                      rx_ok ? static_cast<unsigned>(transfer_size) : 0u,
                      crc_ok ? "PASS" : "FAIL", data_ok ? "PASS" : "FAIL",
                      passed ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("uart_auto", "RX mode=%s seq=%u size=%u cycles=%u status=%s",
                  auto_mode_name(mode), static_cast<unsigned>(sequence),
                  static_cast<unsigned>(transfer_size),
                  static_cast<unsigned>(cycles), passed ? "PASS" : "FAIL");
    }

    void run_cpu_loop_test(Uart& uart, const AutoMode mode,
                           const uint32_t payload_size,
                           const uint16_t sequence,
                           const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        const std::size_t frame_size = dma_frame_size(payload_size);
        char message[224]{};
        if (!prepare_cpu_receive(uart, mode, g_dma_frame, frame_size))
        {
            uart.init_polling();
            uart.put_string("@RESULT mode=loop status=FAIL error=UART_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=%s sequence=%u frame=%u payload=%u\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      static_cast<unsigned>(frame_size),
                      static_cast<unsigned>(payload_size));
        uart.put_string(message);
        uart.wait_tx_complete();
        const bool rx_ok = finish_cpu_receive(uart, mode, g_dma_frame,
                                              frame_size);

        bool header_ok = false;
        bool crc_ok = false;
        bool data_ok = false;
        if (rx_ok)
        {
            const auto* header =
                reinterpret_cast<const DmaFrameHeader*>(g_dma_frame);
            const uint16_t received_crc =
                static_cast<uint16_t>(g_dma_frame[frame_size - 2u]) |
                static_cast<uint16_t>(static_cast<uint16_t>(g_dma_frame[frame_size - 1u]) << 8u);
            header_ok = header->magic == DMA_FRAME_MAGIC &&
                        header->version == DMA_FRAME_VERSION &&
                        header->sequence == sequence &&
                        header->payload_size == payload_size &&
                        header->seed == seed;
            crc_ok = received_crc == crc16_ccitt_false(
                g_dma_frame, frame_size - DMA_FRAME_CRC_SIZE);
            data_ok = header_ok && validate_dma_payload(*header);
        }

        bool tx_ok = false;
        if (rx_ok)
        {
            uart.put_data(g_dma_frame, frame_size);
            uart.wait_tx_complete();
            tx_ok = true;
        }
        uart.init_polling();
        const bool passed = rx_ok && tx_ok && crc_ok && data_ok;
        std::snprintf(message, sizeof(message),
                      "@RESULT mode=%s sequence=%u rx=%u tx=%u crc=%s data=%s status=%s\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      rx_ok ? static_cast<unsigned>(frame_size) : 0u,
                      tx_ok ? static_cast<unsigned>(frame_size) : 0u,
                      crc_ok ? "PASS" : "FAIL", data_ok ? "PASS" : "FAIL",
                      passed ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("uart_auto", "LOOP mode=%s seq=%u frame=%u status=%s",
                  auto_mode_name(mode), static_cast<unsigned>(sequence),
                  static_cast<unsigned>(frame_size), passed ? "PASS" : "FAIL");
    }

    void run_dma_auto_test(Uart& uart,
                           const uint32_t payload_size,
                           const uint16_t sequence,
                           const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        const std::size_t frame_size = dma_frame_size(payload_size);
        char message[192]{};

        RTT_LOG_I("dma_auto", "BEGIN seq=%u payload=%u frame=%u seed=%08x",
                  static_cast<unsigned>(sequence),
                  static_cast<unsigned>(payload_size),
                  static_cast<unsigned>(frame_size),
                  static_cast<unsigned>(seed));

        if (!uart.init_dma())
        {
            RTT_LOG_E("dma_auto", "UART DMA initialization failed");
            uart.init_polling();
            uart.put_string("@RESULT mode=dma status=FAIL error=DMA_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=dma sequence=%u frame=%u payload=%u\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(frame_size),
                      static_cast<unsigned>(payload_size));
        uart.put_string(message);
        uart.wait_tx_complete();

        RTT_LOG_I("dma_auto", "READY sent; entering blocking RX");
        const uint32_t rx_started = HAL::PERF::get_cycle_count();

        if (!uart.read(g_dma_frame, frame_size, TEST_TIMEOUT_LOOPS,
                       TEST_TIMEOUT_LOOPS, dma_timeout_epochs(uart)))
        {
            const uint32_t rx_cycles = HAL::PERF::get_cycle_count() - rx_started;
            RTT_LOG_E("dma_auto", "RX failed after %u PMU cycles",
                      static_cast<unsigned>(rx_cycles));
            log_dma_hw_state("RX_FAIL", REGS::EDMA::CH_UART0_RX);
            uart.init_polling();
            std::snprintf(message, sizeof(message),
                          "@RESULT mode=dma sequence=%u status=FAIL error=RX_TIMEOUT\n",
                          static_cast<unsigned>(sequence));
            uart.put_string(message);
            return;
        }

        const uint32_t rx_cycles = HAL::PERF::get_cycle_count() - rx_started;
        RTT_LOG_I("dma_auto", "RX completed after %u PMU cycles",
                  static_cast<unsigned>(rx_cycles));
        log_dma_hw_state("RX_DONE", REGS::EDMA::CH_UART0_RX);

        const auto* header = reinterpret_cast<const DmaFrameHeader*>(g_dma_frame);
        const uint16_t received_crc =
            static_cast<uint16_t>(g_dma_frame[frame_size - 2u]) |
            static_cast<uint16_t>(static_cast<uint16_t>(g_dma_frame[frame_size - 1u]) << 8u);
        const uint16_t calculated_crc =
            crc16_ccitt_false(g_dma_frame, frame_size - DMA_FRAME_CRC_SIZE);

        const bool header_ok =
            header->magic == DMA_FRAME_MAGIC &&
            header->version == DMA_FRAME_VERSION &&
            header->sequence == sequence &&
            header->payload_size == payload_size &&
            header->seed == seed;
        const bool crc_ok = received_crc == calculated_crc;
        const bool data_ok = header_ok && validate_dma_payload(*header);

        RTT_LOG_I("dma_auto",
                  "VALIDATE header=%s crc=%s data=%s received_crc=%04x calculated_crc=%04x",
                  header_ok ? "PASS" : "FAIL",
                  crc_ok ? "PASS" : "FAIL",
                  data_ok ? "PASS" : "FAIL",
                  static_cast<unsigned>(received_crc),
                  static_cast<unsigned>(calculated_crc));

        // Echo the exact bytes received even on a validation failure.  This lets
        // the host locate and report the first corrupted byte.
        const uint32_t tx_started = HAL::PERF::get_cycle_count();
        const bool tx_ok = uart.write(g_dma_frame, frame_size,
                                      TEST_TIMEOUT_LOOPS,
                                      dma_timeout_epochs(uart));
        const uint32_t tx_cycles = HAL::PERF::get_cycle_count() - tx_started;
        RTT_LOG_I("dma_auto", "TX %s after %u PMU cycles",
                  tx_ok ? "completed" : "failed",
                  static_cast<unsigned>(tx_cycles));
        log_dma_hw_state(tx_ok ? "TX_DONE" : "TX_FAIL", REGS::EDMA::CH_UART0_TX);
        uart.init_polling();

        std::snprintf(message, sizeof(message),
                      "@RESULT mode=dma sequence=%u rx=%u tx=%u crc=%s data=%s status=%s\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(frame_size),
                      tx_ok ? static_cast<unsigned>(frame_size) : 0u,
                      crc_ok ? "PASS" : "FAIL",
                      data_ok ? "PASS" : "FAIL",
                      (tx_ok && crc_ok && data_ok) ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("dma_auto", "END seq=%u status=%s",
                  static_cast<unsigned>(sequence),
                  (tx_ok && crc_ok && data_ok) ? "PASS" : "FAIL");
    }

    void run_timeout_recovery_probe(Uart& uart, const AutoMode mode,
                                    const uint16_t sequence) noexcept
    {
        PollingRestore restore{uart};
        uint8_t probe[DMA_ALIGNMENT]{};
        char message[192]{};
        bool initialized = false;
        bool timed_out = false;

        if (mode == AutoMode::POLLING)
        {
            uart.init_polling();
            initialized = true;
        }
        else if (mode == AutoMode::ISR)
        {
            initialized = start_interrupt_receive(uart, probe, sizeof(probe));
        }
        else
        {
            initialized = uart.init_dma();
        }

        if (!initialized)
        {
            uart.init_polling();
            uart.put_string("@RESULT mode=recover status=FAIL error=UART_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=recover transport=%s sequence=%u stage=timeout\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence));
        uart.put_string(message);
        uart.wait_tx_complete();

        if (mode == AutoMode::POLLING)
        {
            uint32_t remaining = RECOVERY_TIMEOUT_LOOPS;
            while (!uart.rx_data_available() && remaining != 0u)
            {
                --remaining;
                __asm volatile("nop");
            }
            timed_out = remaining == 0u && !uart.rx_data_available();
        }
        else if (mode == AutoMode::ISR)
        {
            uint32_t remaining = RECOVERY_TIMEOUT_LOOPS;
            while (!g_auto_interrupt_complete && remaining != 0u)
            {
                --remaining;
                __asm volatile("nop");
            }
            timed_out = !g_auto_interrupt_complete;
            reset_auto_interrupt_receiver();
        }
        else
        {
            timed_out = !uart.read(probe, sizeof(probe),
                                   RECOVERY_TIMEOUT_LOOPS,
                                   RECOVERY_TIMEOUT_LOOPS, 1u);
        }

        uart.FIFO_clear(false, true);
        uart.init_polling();
        std::snprintf(message, sizeof(message),
                      "@RESULT mode=recover transport=%s sequence=%u timeout=%s status=%s\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      timed_out ? "PASS" : "FAIL",
                      timed_out ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("uart_auto", "RECOVER mode=%s seq=%u timeout=%s",
                  auto_mode_name(mode), static_cast<unsigned>(sequence),
                  timed_out ? "PASS" : "FAIL");
    }

    [[nodiscard]] bool guards_intact(const uint8_t* frame,
                                     const std::size_t frame_size) noexcept
    {
        const auto* begin = g_unaligned_storage;
        const auto* end = g_unaligned_storage + sizeof(g_unaligned_storage);
        for (const uint8_t* cursor = begin; cursor < frame; ++cursor)
            if (*cursor != GUARD_VALUE)
                return false;
        for (const uint8_t* cursor = frame + frame_size; cursor < end; ++cursor)
            if (*cursor != GUARD_VALUE)
                return false;
        return true;
    }

    void run_unaligned_loop_test(Uart& uart, const AutoMode mode,
                                 const uint32_t payload_size,
                                 const uint16_t sequence,
                                 const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        const std::size_t frame_size = dma_frame_size(payload_size);
        for (auto& byte : g_unaligned_storage)
            byte = GUARD_VALUE;
        uint8_t* const frame = g_unaligned_storage + GUARD_SIZE + 1u;
        char message[224]{};
        bool initialized = false;

        if (mode == AutoMode::POLLING)
        {
            uart.init_polling();
            initialized = true;
        }
        else if (mode == AutoMode::ISR)
        {
            initialized = start_interrupt_receive(uart, frame, frame_size);
        }
        else
        {
            initialized = uart.init_dma();
        }

        if (!initialized)
        {
            uart.init_polling();
            uart.put_string("@RESULT mode=offset status=FAIL error=UART_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=offset transport=%s sequence=%u frame=%u payload=%u offset=1\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      static_cast<unsigned>(frame_size),
                      static_cast<unsigned>(payload_size));
        uart.put_string(message);
        uart.wait_tx_complete();

        const bool rx_ok = mode == AutoMode::DMA
            ? uart.read(frame, frame_size, TEST_TIMEOUT_LOOPS,
                        TEST_TIMEOUT_LOOPS, dma_timeout_epochs(uart))
            : (mode == AutoMode::POLLING
                ? receive_polling(uart, frame, frame_size)
                : wait_interrupt_receive(uart, frame_size));

        DmaFrameHeader header{};
        auto* header_bytes = reinterpret_cast<uint8_t*>(&header);
        for (std::size_t i = 0u; i < sizeof(header); ++i)
            header_bytes[i] = frame[i];
        const bool header_ok = rx_ok &&
            header.magic == DMA_FRAME_MAGIC &&
            header.version == DMA_FRAME_VERSION &&
            header.sequence == sequence &&
            header.payload_size == payload_size && header.seed == seed;
        const uint16_t received_crc = rx_ok
            ? static_cast<uint16_t>(frame[frame_size - 2u]) |
              static_cast<uint16_t>(static_cast<uint16_t>(frame[frame_size - 1u]) << 8u)
            : 0u;
        const bool crc_ok = rx_ok && received_crc ==
            crc16_ccitt_false(frame, frame_size - DMA_FRAME_CRC_SIZE);
        uint32_t state = seed != 0u ? seed : 0x6D2B79F5u;
        bool data_ok = header_ok;
        for (std::size_t i = 0u; i < payload_size && data_ok; ++i)
            data_ok = frame[DMA_FRAME_HEADER_SIZE + i] ==
                static_cast<uint8_t>(xorshift32(state) & 0xFFu);
        bool guard_ok = guards_intact(frame, frame_size);

        bool tx_ok = false;
        if (rx_ok)
        {
            if (mode == AutoMode::DMA)
                tx_ok = uart.write(frame, frame_size, TEST_TIMEOUT_LOOPS,
                                   dma_timeout_epochs(uart));
            else
            {
                uart.put_data(frame, frame_size);
                uart.wait_tx_complete();
                tx_ok = true;
            }
        }
        guard_ok = guard_ok && guards_intact(frame, frame_size);
        uart.init_polling();
        const bool passed = rx_ok && tx_ok && crc_ok && data_ok && guard_ok;
        std::snprintf(message, sizeof(message),
                      "@RESULT mode=offset transport=%s sequence=%u rx=%u tx=%u crc=%s data=%s guard=%s status=%s\n",
                      auto_mode_name(mode), static_cast<unsigned>(sequence),
                      rx_ok ? static_cast<unsigned>(frame_size) : 0u,
                      tx_ok ? static_cast<unsigned>(frame_size) : 0u,
                      crc_ok ? "PASS" : "FAIL", data_ok ? "PASS" : "FAIL",
                      guard_ok ? "PASS" : "FAIL", passed ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("uart_auto", "OFFSET mode=%s seq=%u frame=%u guard=%s status=%s",
                  auto_mode_name(mode), static_cast<unsigned>(sequence),
                  static_cast<unsigned>(frame_size),
                  guard_ok ? "PASS" : "FAIL", passed ? "PASS" : "FAIL");
    }

    void print_test_result(const Uart& uart, const char* name, const bool passed) noexcept
    {
        uart.put_string("[RESULT] ");
        uart.put_string(name);
        uart.put_string(passed ? ": PASS\n" : ": FAIL\n");

        if (passed)
            RTT_LOG_I(TAG, "%s test passed", name);
        else
            RTT_LOG_E(TAG, "%s test failed", name);
    }

    void print_help_impl(const Uart& uart) noexcept
    {
        uart.put_string(
            "\nCommands:\n"
            "  help             Show this help\n"
            "  test polling     Polling TX + polling RX\n"
            "  test interrupt   Polling TX + ISR RX\n"
            "  test dma         EDMA TX + EDMA RX\n"
            "  test all         Run all three tests\n"
            "\nEditing:\n"
            "  Up/Down          Command history\n"
            "  Left/Right       Move cursor\n"
            "  Home/End         Start/end of line\n"
            "  Backspace/Delete Edit line\n"
            "  Ctrl+C           Cancel line\n"
            "  Ctrl+L           Clear screen\n"
            "  Tab              Complete command\n");
    }

    void run_all_tests(Uart& uart) noexcept
    {
        const bool polling_passed = UART_TESTS::run_polling(uart);
        print_test_result(uart, "polling", polling_passed);
        const bool interrupt_passed = UART_TESTS::run_interrupt(uart);
        print_test_result(uart, "interrupt (ISR RX + polling TX)", interrupt_passed);
        const bool dma_passed = UART_TESTS::run_dma(uart);
        print_test_result(uart, "dma", dma_passed);

        uart.put_string("\n[SUMMARY]\n");
        uart.put_string(polling_passed ? "  polling:   PASS\n" : "  polling:   FAIL\n");
        uart.put_string(interrupt_passed ? "  interrupt: PASS\n" : "  interrupt: FAIL\n");
        uart.put_string(dma_passed ? "  dma:       PASS\n" : "  dma:       FAIL\n");
        uart.put_string(polling_passed && interrupt_passed && dma_passed
                            ? "  overall:   PASS\n"
                            : "  overall:   FAIL\n");
    }

    void execute_command(Uart& uart, const char* command) noexcept
    {
        uint32_t payload_size = 0u;
        uint32_t sequence = 0u;
        uint32_t seed = 0u;
        uint32_t baud_index = 0u;
        AutoMode auto_mode = AutoMode::DMA;

        if (strings_equal(command, "help"))
            print_help_impl(uart);
        else if (strings_equal(command, "test polling"))
            print_test_result(uart, "polling", UART_TESTS::run_polling(uart));
        else if (strings_equal(command, "test interrupt"))
            print_test_result(uart, "interrupt (ISR RX + polling TX)", UART_TESTS::run_interrupt(uart));
        else if (strings_equal(command, "test dma"))
            print_test_result(uart, "dma", UART_TESTS::run_dma(uart));
        else if (strings_equal(command, "test all"))
            run_all_tests(uart);
        else if (parse_auto_dma(command, auto_mode, payload_size, sequence, seed))
        {
            if (payload_size == 0u || payload_size > DMA_MAX_PAYLOAD_SIZE ||
                sequence > 0xFFFFu)
            {
                uart.put_string("@RESULT mode=loop status=FAIL error=BAD_ARGUMENT\n");
            }
            else
            {
                if (auto_mode == AutoMode::DMA)
                    run_dma_auto_test(uart, payload_size,
                                      static_cast<uint16_t>(sequence), seed);
                else
                    run_cpu_loop_test(uart, auto_mode, payload_size,
                                      static_cast<uint16_t>(sequence), seed);
            }
        }
        else if (parse_auto_tx(command, auto_mode, payload_size, sequence, seed))
        {
            if (payload_size < DMA_FRAME_CRC_SIZE ||
                payload_size > DMA_MAX_FRAME_SIZE || sequence > 0xFFFFu)
            {
                uart.put_string("@RESULT mode=tx status=FAIL error=BAD_ARGUMENT\n");
            }
            else
            {
                if (auto_mode == AutoMode::DMA)
                    run_tx_tail_test(uart, payload_size,
                                     static_cast<uint16_t>(sequence), seed);
                else
                    run_cpu_tx_test(uart, auto_mode, payload_size,
                                    static_cast<uint16_t>(sequence), seed);
            }
        }
        else if (parse_auto_rx(command, auto_mode, payload_size, sequence, seed))
        {
            if (payload_size < DMA_FRAME_CRC_SIZE ||
                payload_size > DMA_MAX_FRAME_SIZE || sequence > 0xFFFFu)
            {
                uart.put_string("@RESULT mode=rx status=FAIL error=BAD_ARGUMENT\n");
            }
            else
            {
                if (auto_mode == AutoMode::DMA)
                    run_rx_tail_test(uart, payload_size,
                                     static_cast<uint16_t>(sequence), seed);
                else
                    run_cpu_rx_test(uart, auto_mode, payload_size,
                                    static_cast<uint16_t>(sequence), seed);
            }
        }
        else if (parse_auto_recover(command, auto_mode, payload_size,
                                    sequence, seed))
        {
            if (payload_size == 0u || payload_size > DMA_MAX_PAYLOAD_SIZE ||
                sequence > 0xFFFFu)
            {
                uart.put_string("@RESULT mode=recover status=FAIL error=BAD_ARGUMENT\n");
            }
            else
            {
                run_timeout_recovery_probe(uart, auto_mode,
                                           static_cast<uint16_t>(sequence));
            }
        }
        else if (parse_auto_offset(command, auto_mode, payload_size,
                                   sequence, seed))
        {
            if (payload_size == 0u || payload_size > DMA_MAX_PAYLOAD_SIZE ||
                sequence > 0xFFFFu)
            {
                uart.put_string("@RESULT mode=offset status=FAIL error=BAD_ARGUMENT\n");
            }
            else
            {
                run_unaligned_loop_test(uart, auto_mode, payload_size,
                                        static_cast<uint16_t>(sequence), seed);
            }
        }
        else if (parse_auto_baud(command, baud_index))
        {
            if (baud_index >= UART_BAUD_COUNT)
                uart.put_string("@BAUD ERROR error=BAD_ARGUMENT\n");
            else
                run_baud_switch(uart, baud_index);
        }
        else if (command[0] == 'a' && command[1] == 'u' &&
                 command[2] == 't' && command[3] == 'o')
            uart.put_string("@RESULT mode=auto status=FAIL error=BAD_COMMAND\n");
        else if (*command != '\0')
            uart.put_string("Unknown command. Type 'help'.\n");
    }

    void dispatch(void* context, const char* command) noexcept
    {
        execute_command(*static_cast<Uart*>(context), command);
    }
}

namespace UART_TESTS
{
    void print_help(Uart& uart) noexcept
    {
        print_help_impl(uart);
    }

    void register_commands(SHELL::Shell& shell, Uart& uart) noexcept
    {
        using Visibility = SHELL::Shell::Visibility;
        (void)shell.add_command("help", dispatch, &uart);
        (void)shell.add_command("test polling", dispatch, &uart);
        (void)shell.add_command("test interrupt", dispatch, &uart);
        (void)shell.add_command("test dma", dispatch, &uart);
        (void)shell.add_command("test all", dispatch, &uart);
        (void)shell.add_command("auto", dispatch, &uart, Visibility::HIDDEN);
    }
}
