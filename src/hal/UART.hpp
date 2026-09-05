#ifndef _HAL_UART_HPP
#define _HAL_UART_HPP

#include <cstdint>
#include "../regs/REGS.hpp"
#include "INTC.hpp"
#include "pins/am3359_pins.hpp"

#if defined(__GNUC__)
#pragma GCC push_options
#pragma GCC optimize ("-O0")
#endif

namespace HAL::UART
{
    typedef void (*serial_user_callback)(char);

    enum class IOMode : uint8_t
    {
        UNINITIALIZED = 0,
        POLLING,
        INTERRUPT,
        DMA
    };

    struct DMAConfig
    {
        uint8_t tx_chunk {8u};
        uint8_t rx_chunk {8u};
        uint8_t tx_trigger_space {8u};
        uint8_t rx_trigger_level {8u};
        uint8_t tx_dma_threshold {5u};
        REGS::UART::e_SCR_DMA_MODE mode {REGS::UART::SCR_DMA_MODE_1};
    };

}

#include "hal/EDMA/periph/UartBackend.hpp"

namespace HAL::UART
{

    class uart_base
    {
    protected:
        REGS::UART::AM335x_UART_Type& m_instance;

        REGS::UART::LCR_reg_t m_LCR_before;
        REGS::UART::FCR_reg_t m_FCR_shadow;
        REGS::UART::e_BAUDRATE m_baudrate{REGS::UART::KBPS_115_2};
        REGS::UART::e_MODESELECT m_baud_mode{REGS::UART::MODE_UART_16x};
        [[gnu::always_inline]] void m_save_LCR() noexcept           { m_LCR_before.reg = m_instance.LCR.reg; }
        [[gnu::always_inline]] void m_restore_LCR() const noexcept  { m_instance.LCR.reg = m_LCR_before.reg; }

        explicit uart_base(REGS::UART::AM335x_UART_Type* uart_regs)
            : m_instance(*uart_regs),
              m_LCR_before{},
              m_FCR_shadow{}
        {}

    public:
        /// <--- Start module init ---> ///
        void reset_module() const noexcept;
        void resume_operation() const noexcept;

        /// <--- Mode select methods  TRM 19.3 ---> ///
        void switch_operating_mode(REGS::UART::e_MODESELECT mode) const noexcept;
        [[nodiscard]] REGS::UART::e_MODESELECT get_current_operating_mode() const noexcept;
        void switch_reg_config_mode(REGS::UART::e_CONFIG_MODE mode, REGS::UART::e_ENH enh) const noexcept;
        [[nodiscard]] REGS::UART::e_CONFIG_MODE get_config_mode() const noexcept;
        void switch_reg_subconfig_mode(REGS::UART::e_SUBCONFIG_MODE mode) noexcept;

        // Базовые операции
        void put_char(char c) const noexcept;
        [[nodiscard]] char get_char() const noexcept;
        [[nodiscard]] bool rx_data_available() const noexcept;
        void put_string(const char* str) const noexcept;
        void wait_tx_complete() const noexcept;
        [[nodiscard]] bool tx_fifo_full() const noexcept;
        [[nodiscard]] bool tx_fifo_empty() const noexcept;
        [[nodiscard]] bool tx_busy() const noexcept;
        void put_data(const void* data, std::size_t size) const noexcept;
        // Bounded polling TX for diagnostics. Caller must own the UART TX path.
        [[nodiscard]] bool put_data_bounded(const void* data, std::size_t size,
                                            uint32_t timeout_loops) const noexcept;

        /// <--- FIFO management methods TRM 19.3 ---> ///
        [[gnu::noinline]] void FIFO_register_write(REGS::UART::FCR_reg_t fcr) noexcept;
        [[nodiscard]] bool FIFO_configure(uint8_t tx_trigger_level,
                                          uint8_t rx_trigger_level,
                                          REGS::UART::e_SCR_DMA_MODE dma_mode) noexcept;
        void FIFO_clear(bool clear_tx = true, bool clear_rx = true) noexcept;

        /// <--- DMA request management ---> ///
        void DMA_enable(REGS::UART::e_SCR_DMA_MODE mode) noexcept;
        void DMA_disable() noexcept;
        [[nodiscard]] bool TX_DMA_threshold_configure(uint8_t threshold) noexcept;

        /// <--- Protocol formating methods TRM 19.3 ---> ///
        ///  1. Clock generation setup:
        [[nodiscard]]REGS::UART::divisor_latch divisor_latch_get() noexcept;
        [[gnu::noinline]] void divisor_latch_set(REGS::UART::divisor_latch divisor) noexcept;
        void divisor_latch_enable() noexcept;
        void divisor_latch_disable() noexcept;
        [[gnu::noinline]] void config_baudrate() noexcept;

        ///  2. Data formating setup:
        void data_format_set(REGS::UART::e_CHAR_LENGHT char_len,
                            REGS::UART::e_STOP_BIT stop_bit,
                            REGS::UART::e_LCR_PARITY parity) noexcept;

        ///  3. Interrupt management:
        void int_enable(REGS::UART::e_UART_IT_EN int_flag) noexcept;
        void int_disable(REGS::UART::e_UART_IT_EN int_flag) noexcept;

        /// <--- Power management methods ---> ///
        void idle_mode_configure(REGS::UART::e_IDLEMODE mode) noexcept;
        void wakeup_control(bool control) noexcept;
        void auto_idle_mode_control(bool control) noexcept;
        void sleep(bool control) noexcept;

        /// <--- Other management methods ---> ///
        void modem_control_set(REGS::UART::MCR_reg_t mcr) noexcept;

        ~uart_base() = default;
        uart_base(const uart_base&) = delete;
        uart_base& operator=(const uart_base&) = delete;
        uart_base(uart_base&&) = default;
        uart_base& operator=(uart_base&&) = delete;
    };

        template <typename Derived,
                  typename TXPin,
                  typename RXPin,
                  uint32_t UARTBase,
                  uint32_t IRQNum,
                  uint8_t TxDmaChannel = 0xFFu,
                  uint8_t RxDmaChannel = 0xFFu,
                  uint8_t DummyParam = 63u>
    class uart : protected uart_base
    {
    protected:
    using TXMode = typename TXPin::pinmode_type;
    using RXMode = typename RXPin::pinmode_type;

    volatile TXPin& m_tx;
    volatile RXPin& m_rx;
    TXMode m_tx_mode;
    RXMode m_rx_mode;

    static serial_user_callback m_user_callback;
    static uart* m_active_instance;
    IOMode m_io_mode{IOMode::UNINITIALIZED};
    using DmaBackend = EDMA::periph::UartBackend<uart, TxDmaChannel,
                                                 RxDmaChannel, DummyParam>;
    friend DmaBackend;
    DmaBackend m_dma{*this};

    uint8_t* volatile m_rx_buffer{nullptr};
    volatile std::size_t m_rx_expected{0u};
    volatile std::size_t m_rx_count{0u};
    volatile bool m_rx_active{false};
    volatile bool m_rx_complete{false};
    volatile bool m_rx_success{false};

    // CRTP access to base class
    Derived& derived() { return static_cast<Derived&>(*this); }

    static void uart_isr(void*)
    {
        if (m_active_instance != nullptr)
            m_active_instance->handle_uart_irq();
        REGS::INTC::new_IRQ_agree();
    }

    void handle_uart_irq() noexcept
    {
        const auto type = static_cast<REGS::UART::e_IT_TYPES>(
            m_instance.IIR_UART.b.IT_TYPE);
        const bool hardware_timeout = type == REGS::UART::RX_TOUT_IT;
        if (type != REGS::UART::RHR_IT && !hardware_timeout)
        {
            if (type == REGS::UART::RX_LINE_STS_ERR)
                (void)m_instance.LSR_UART.reg;
            return;
        }

        // RXFIFO_LVL is a stable snapshot of the number of readable bytes.
        // Re-checking LSR.RXFIFOE after every RHR access can over-read once at
        // the empty boundary, especially when the character timeout fires at
        // high baud rates.
        std::size_t fifo_level =
            static_cast<std::size_t>(m_instance.RXFIFO_LVL.b.RXFIFO_LVL);
        while (fifo_level != 0u)
        {
            --fifo_level;
            const uint8_t value = static_cast<uint8_t>(m_instance.RHR.b.RHR);
            if (m_rx_active && m_rx_buffer != nullptr)
            {
                const std::size_t index = m_rx_count;
                if (index < m_rx_expected)
                {
                    m_rx_buffer[index] = value;
                    m_rx_count = index + 1u;
                }
                if (m_rx_count == m_rx_expected)
                {
                    m_rx_active = false;
                    m_rx_success = true;
                    // Publish every buffer store before the completion flag.
                    // volatile alone is not a release barrier on Cortex-A8.
                    cp15_DMB_barrier();
                    m_rx_complete = true;
                }
            }
            else if (m_user_callback != nullptr)
            {
                m_user_callback(static_cast<char>(value));
            }
        }
    }

    void init_pins() noexcept
    {
        using namespace REGS::CONTROL_MODULE;

        m_rx.gpio_module_init();
        m_rx.sel_pinmode(m_rx_mode);
        m_rx.pullup_enable(true);
        m_rx.sel_pull_type(PULL_UP);
        m_rx.dir_set(REGS::GPIO::GPIO_INPUT);
        m_rx.sel_slewrate(FAST);

        m_tx.sel_pinmode(m_tx_mode);
        m_tx.pullup_enable(true);
        m_tx.sel_pull_type(PULL_UP);
        m_tx.dir_set(REGS::GPIO::GPIO_OUTPUT);
        m_tx.sel_slewrate(FAST);
    }

    void setup_interrupts(serial_user_callback cb) noexcept
    {
        using namespace REGS::UART;
        using namespace REGS::INTC;

        m_active_instance = this;
        m_user_callback = cb;
        INTC::register_handler(static_cast<e_INT_ID>(IRQNum), uart_isr);
        int_enable(RECEIVE_IT);
        switch_reg_config_mode(OPERATIONAL_MODE, ENH_DISABLE);
        switch_operating_mode(m_baud_mode);
        resume_operation();
        INTC::unmask_interrupt(static_cast<e_INT_ID>(IRQNum));
    }

    void cleanup_interrupts() noexcept
    {
        INTC::mask_interrupt(static_cast<REGS::INTC::e_INT_ID>(IRQNum));
        int_disable(REGS::UART::RECEIVE_IT);
        m_user_callback = nullptr;
        m_active_instance = nullptr;
        reset_rx_transaction();
    }

    void reset_rx_transaction() noexcept
    {
        m_rx_buffer = nullptr;
        m_rx_expected = 0u;
        m_rx_count = 0u;
        m_rx_active = false;
        m_rx_complete = false;
        m_rx_success = false;
    }

    [[nodiscard]] bool arm_interrupt_receive(void* data,
                                             const std::size_t size) noexcept
    {
        if ((data == nullptr && size != 0u) || size == 0u)
            return size == 0u;
        reset_rx_transaction();
        m_rx_buffer = static_cast<uint8_t*>(data);
        m_rx_expected = size;
        m_rx_active = true;
        return true;
    }

    [[nodiscard]] bool wait_interrupt_receive(
        const std::size_t size, const uint32_t timeout_loops) noexcept
    {
        uint32_t remaining = timeout_loops;
        while (!m_rx_complete && remaining != 0u)
        {
            --remaining;
            __asm volatile("nop");
        }
        // Pair with the ISR release barrier before consuming the buffer.
        cp15_DMB_barrier();
        if (!m_rx_complete)
        {
            m_rx_active = false;
            m_rx_success = false;
        }
        const bool success = m_rx_success && m_rx_count == size;
        reset_rx_transaction();
        return success;
    }

    [[nodiscard]] bool receive_by_interrupt(void* data,
                                            const std::size_t size,
                                            const uint32_t timeout_loops) noexcept
    {
        return arm_interrupt_receive(data, size) &&
               wait_interrupt_receive(size, timeout_loops);
    }

    void finish_common_init() noexcept
    {
        switch_reg_config_mode(REGS::UART::OPERATIONAL_MODE, REGS::UART::ENH_DISABLE);
        switch_operating_mode(m_baud_mode);
        resume_operation();
    }

    public:
            uart(TXPin& tx, RXPin& rx, TXMode tx_mode, RXMode rx_mode)
            : uart_base(reinterpret_cast<REGS::UART::AM335x_UART_Type*>(UARTBase))
            , m_tx(tx)
            , m_rx(rx)
            , m_tx_mode(tx_mode)
            , m_rx_mode(rx_mode)
            { }

            void init_polling() noexcept
            {
                deinit();
                derived().run_clocks();
                m_save_LCR();
                init_pins();
                config_baudrate();
                DMA_disable();
                int_disable(static_cast<REGS::UART::e_UART_IT_EN>(0xFFu));
                finish_common_init();
                m_io_mode = IOMode::POLLING;
            }

            [[nodiscard]] bool set_baudrate(const REGS::UART::e_BAUDRATE baudrate) noexcept
            {
                if (static_cast<uint32_t>(baudrate) >
                    static_cast<uint32_t>(REGS::UART::MBPS_3_6884))
                {
                    return false;
                }
                m_baudrate = baudrate;
                init_polling();
                return true;
            }

            [[nodiscard]] REGS::UART::e_BAUDRATE baudrate() const noexcept
            {
                return m_baudrate;
            }

            [[nodiscard]] bool init_interrupt(serial_user_callback cb) noexcept
            {
                if (cb == nullptr)
                    return false;

                init_polling();
                setup_interrupts(cb);
                m_io_mode = IOMode::INTERRUPT;
                return true;
            }

            [[nodiscard]] bool init_dma(const DMAConfig& config = {}) noexcept
            {
                if constexpr (TxDmaChannel == 0xFFu || RxDmaChannel == 0xFFu)
                    return false;
                deinit();
                derived().run_clocks();
                m_save_LCR();
                init_pins();
                config_baudrate();
                int_disable(static_cast<REGS::UART::e_UART_IT_EN>(0xFFu));
                INTC::mask_interrupt(static_cast<REGS::INTC::e_INT_ID>(IRQNum));

                if (!FIFO_configure(config.tx_trigger_space,
                                    config.rx_trigger_level,
                                    config.mode) ||
                    !TX_DMA_threshold_configure(config.tx_dma_threshold) ||
                    !m_dma.init(config))
                {
                    m_dma.stop();
                    init_polling();
                    return false;
                }

                finish_common_init();
                m_io_mode = IOMode::DMA;
                return true;
            }

            // Backwards-compatible entry point used by existing bootloader code.
            void init(serial_user_callback cb = nullptr) noexcept
            {
                if (cb != nullptr)
                    (void)init_interrupt(cb);
                else
                    init_polling();
            }

            void deinit() noexcept
            {
                // A freshly constructed UART has no functional clock yet.
                // Teardown must not touch its MMIO registers in that state.
                if (m_io_mode == IOMode::UNINITIALIZED)
                    return;

                if (m_io_mode == IOMode::INTERRUPT)
                    cleanup_interrupts();
                else
                    INTC::mask_interrupt(static_cast<REGS::INTC::e_INT_ID>(IRQNum));

                m_dma.stop();
                DMA_disable();
                reset_rx_transaction();
                m_io_mode = IOMode::UNINITIALIZED;
            }

            ~uart() noexcept { deinit(); }

            [[nodiscard]] IOMode io_mode() const noexcept { return m_io_mode; }
            [[nodiscard]] static constexpr uintptr_t tx_dma_address() noexcept { return UARTBase; }
            [[nodiscard]] static constexpr uintptr_t rx_dma_address() noexcept { return UARTBase; }

            [[nodiscard]] bool write(const void* data, const std::size_t size,
                                     const uint32_t timeout_loops = 5'000'000u,
                                     const uint32_t timeout_epochs = 1u) noexcept
            {
                if (m_io_mode == IOMode::UNINITIALIZED ||
                    (data == nullptr && size != 0u))
                    return false;
                if (m_io_mode == IOMode::DMA)
                    return m_dma.write(data, size, timeout_loops,
                                       timeout_epochs);
                put_data(data, size);
                return true;
            }

            [[nodiscard]] bool read(void* data, const std::size_t size,
                                    const uint32_t timeout_loops = 5'000'000u,
                                    const uint32_t tail_timeout_loops = 5'000'000u,
                                    const uint32_t timeout_epochs = 1u) noexcept
            {
                if (m_io_mode == IOMode::UNINITIALIZED ||
                    (data == nullptr && size != 0u))
                    return false;
                if (size == 0u)
                    return true;
                if (m_io_mode == IOMode::DMA)
                    return m_dma.read(data, size, timeout_loops,
                                      tail_timeout_loops, timeout_epochs);
                if (m_io_mode == IOMode::INTERRUPT)
                    return receive_by_interrupt(data, size, timeout_loops);

                auto* bytes = static_cast<uint8_t*>(data);
                for (std::size_t i = 0u; i < size; ++i)
                {
                    uint32_t remaining = timeout_loops;
                    while (!rx_data_available() && remaining != 0u)
                    {
                        --remaining;
                        __asm volatile("nop");
                    }
                    if (remaining == 0u)
                        return false;
                    bytes[i] = static_cast<uint8_t>(get_char());
                }
                return true;
            }

            [[nodiscard]] bool transmit_dma(
                const void* data, const std::size_t size,
                const uint32_t timeout_loops = 5'000'000u,
                const uint32_t timeout_epochs = 1u) noexcept
            {
                return m_io_mode == IOMode::DMA &&
                       m_dma.write(data, size, timeout_loops, timeout_epochs);
            }

            [[nodiscard]] bool receive_dma(
                void* data, const std::size_t size,
                const uint32_t timeout_loops = 5'000'000u,
                const uint32_t tail_timeout_loops = 5'000'000u,
                const uint32_t timeout_epochs = 1u) noexcept
            {
                return m_io_mode == IOMode::DMA &&
                       m_dma.read(data, size, timeout_loops,
                                  tail_timeout_loops, timeout_epochs);
            }

            // Экспортируем нужные методы из uart_core
            using uart_base::put_char;
            using uart_base::get_char;
            using uart_base::rx_data_available;
            using uart_base::put_string;
            using uart_base::DMA_enable;
            using uart_base::DMA_disable;
            using uart_base::FIFO_clear;
            using uart_base::put_data;
            using uart_base::put_data_bounded;
            using uart_base::wait_tx_complete;
    };

    template <typename Derived, typename TXPin, typename RXPin,
              uint32_t UARTBase, uint32_t IRQNum, uint8_t TxDmaChannel,
              uint8_t RxDmaChannel, uint8_t DummyParam>
    serial_user_callback uart<Derived, TXPin, RXPin, UARTBase, IRQNum,
                              TxDmaChannel, RxDmaChannel,
                              DummyParam>::m_user_callback = nullptr;

    template <typename Derived, typename TXPin, typename RXPin,
              uint32_t UARTBase, uint32_t IRQNum, uint8_t TxDmaChannel,
              uint8_t RxDmaChannel, uint8_t DummyParam>
    uart<Derived, TXPin, RXPin, UARTBase, IRQNum, TxDmaChannel,
         RxDmaChannel, DummyParam>*
    uart<Derived, TXPin, RXPin, UARTBase, IRQNum, TxDmaChannel,
         RxDmaChannel, DummyParam>::m_active_instance = nullptr;

    class uart0_t : public uart<uart0_t,
                          HAL::PINS::UART0_TXD,
                          HAL::PINS::UART0_RXD,
                          REGS::UART::AM335x_UART_0_BASE,
                          REGS::INTC::UART0INT,
                          REGS::EDMA::CH_UART0_TX,
                          REGS::EDMA::CH_UART0_RX>
    {
        using Base = uart<uart0_t,
                          HAL::PINS::UART0_TXD,
                          HAL::PINS::UART0_RXD,
                          REGS::UART::AM335x_UART_0_BASE,
                          REGS::INTC::UART0INT,
                          REGS::EDMA::CH_UART0_TX,
                          REGS::EDMA::CH_UART0_RX>;
        friend class uart<uart0_t,
                          HAL::PINS::UART0_TXD,
                          HAL::PINS::UART0_RXD,
                          REGS::UART::AM335x_UART_0_BASE,
                          REGS::INTC::UART0INT,
                          REGS::EDMA::CH_UART0_TX,
                          REGS::EDMA::CH_UART0_RX>;

        static void run_clocks() noexcept
        {
            using namespace REGS::PRCM;
            volatile auto& wkup = *AM335x_CM_WKUP;

            wkup.UART0_CLKCTRL.b.MODULEMODE = MODULEMODE_DISABLED;
            wkup.UART0_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
            while(wkup.UART0_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}

            wkup.L4WKUP_CLKCTRL.b.MODULEMODE = MODULEMODE_DISABLED;
            wkup.L4WKUP_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
            while(wkup.L4WKUP_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}
        }

    public:
        uart0_t(volatile HAL::PINS::UART0_TXD& tx, volatile HAL::PINS::UART0_RXD& rx)
            : Base(const_cast<HAL::PINS::UART0_TXD&>(tx),
                   const_cast<HAL::PINS::UART0_RXD&>(rx),
                   HAL::PINS::e_UART0_TXD::uart0_txd,
                   HAL::PINS::e_UART0_RXD::uart0_rxd)
        {}
    };
/*
    class uart1_t : public uart<uart1_t,
                          HAL::PINS::UART1_TXD,
                          HAL::PINS::UART1_RXD,
                          REGS::UART::AM335x_UART_1_BASE,
                          REGS::INTC::UART1INT>
    {
        using Base = uart<uart1_t,
                          HAL::PINS::UART1_TXD,
                          HAL::PINS::UART1_RXD,
                          REGS::UART::AM335x_UART_1_BASE,
                          REGS::INTC::UART1INT>;
        friend class uart<uart1_t,
                          HAL::PINS::UART1_TXD,
                          HAL::PINS::UART1_RXD,
                          REGS::UART::AM335x_UART_1_BASE,
                          REGS::INTC::UART1INT>;

        static void run_clocks() noexcept
        {
            using namespace REGS::PRCM;
            volatile auto& per = *AM335x_CM_PER;

            per.UART1_CLKCTRL.b.MODULEMODE = MODULEMODE_DISABLED;
            per.UART1_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
            while(per.UART1_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}

            per.L4LS_CLKCTRL.b.MODULEMODE = MODULEMODE_DISABLED;
            per.L4LS_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
            while(per.L4LS_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}
            while(per.L4LS_CLKSTCTRL.b.CLKACTIVITY_UART_GFCLK != CLK_ACT) {}
        }

    public:
        uart1_t(volatile HAL::PINS::UART1_TXD& tx, volatile HAL::PINS::UART1_RXD& rx)
            : Base(const_cast<HAL::PINS::UART1_TXD&>(tx),
                  const_cast<HAL::PINS::UART1_RXD&>(rx),
                   HAL::PINS::e_UART1_TXD::uart1_txd,
                   HAL::PINS::e_UART1_RXD::uart1_rxd)
        {}
    };
*/
}

#if defined(__GNUC__)
#pragma GCC pop_options
#endif

#endif //_HAL_UART_HPP
