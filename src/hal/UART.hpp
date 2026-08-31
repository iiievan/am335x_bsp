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
        uint8_t tx_trigger_space {8u};
        uint8_t rx_trigger_level {8u};
        uint8_t tx_dma_threshold {5u};
        REGS::UART::e_SCR_DMA_MODE mode {REGS::UART::SCR_DMA_MODE_1};
    };

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
                  uint32_t IRQNum>
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
    IOMode m_io_mode{IOMode::UNINITIALIZED};

    // CRTP access to base class
    Derived& derived() { return static_cast<Derived&>(*this); }

    static void uart_isr(void*)
    {
        auto* uart = reinterpret_cast<REGS::UART::AM335x_UART_Type*>(UARTBase);
        char c = uart->RHR.reg;
        if (m_user_callback) m_user_callback(c);
        REGS::INTC::new_IRQ_agree();
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

        int_enable(RECEIVE_IT);  // Из uart_core
        switch_reg_config_mode(OPERATIONAL_MODE, ENH_DISABLE);  // Из uart_core

        switch_operating_mode(m_baud_mode);  // Из uart_core
        resume_operation();  // Из uart_core

        m_user_callback = cb;

        // Регистрируем обработчик в контроллере прерываний
        INTC::register_handler(static_cast<e_INT_ID>(IRQNum), uart_isr);
        INTC::unmask_interrupt(static_cast<e_INT_ID>(IRQNum));
    }

    void cleanup_interrupts() noexcept
    {
        INTC::mask_interrupt(static_cast<REGS::INTC::e_INT_ID>(IRQNum));
        int_disable(REGS::UART::RECEIVE_IT);
        m_user_callback = nullptr;
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
                    !TX_DMA_threshold_configure(config.tx_dma_threshold))
                {
                    DMA_disable();
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
                if (m_io_mode == IOMode::UNINITIALIZED)
                    return;

                if (m_io_mode == IOMode::INTERRUPT)
                    cleanup_interrupts();
                else
                    INTC::mask_interrupt(static_cast<REGS::INTC::e_INT_ID>(IRQNum));

                DMA_disable();
                m_io_mode = IOMode::UNINITIALIZED;
            }

            ~uart() noexcept { deinit(); }

            [[nodiscard]] IOMode io_mode() const noexcept { return m_io_mode; }
            [[nodiscard]] static constexpr uintptr_t tx_dma_address() noexcept { return UARTBase; }
            [[nodiscard]] static constexpr uintptr_t rx_dma_address() noexcept { return UARTBase; }

            // Экспортируем нужные методы из uart_core
            using uart_base::put_char;
            using uart_base::get_char;
            using uart_base::rx_data_available;
            using uart_base::put_string;
            using uart_base::DMA_enable;
            using uart_base::DMA_disable;
            using uart_base::FIFO_clear;
            using uart_base::put_data;
            using uart_base::wait_tx_complete;
    };

    template <typename Derived, typename TXPin, typename RXPin, uint32_t UARTBase, uint32_t IRQNum>
    serial_user_callback uart<Derived, TXPin, RXPin, UARTBase, IRQNum>::m_user_callback = nullptr;

    class uart0_t : public uart<uart0_t,
                          HAL::PINS::UART0_TXD,
                          HAL::PINS::UART0_RXD,
                          REGS::UART::AM335x_UART_0_BASE,
                          REGS::INTC::UART0INT>
    {
        using Base = uart<uart0_t,
                          HAL::PINS::UART0_TXD,
                          HAL::PINS::UART0_RXD,
                          REGS::UART::AM335x_UART_0_BASE,
                          REGS::INTC::UART0INT>;
        friend class uart<uart0_t,
                          HAL::PINS::UART0_TXD,
                          HAL::PINS::UART0_RXD,
                          REGS::UART::AM335x_UART_0_BASE,
                          REGS::INTC::UART0INT>;

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
