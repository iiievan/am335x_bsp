#ifndef HAL_MMCSD_MMCSD_HPP
#define HAL_MMCSD_MMCSD_HPP

#include <cstddef>
#include <cstdint>

#include "regs/REGS.hpp"
#include "hal/pins/am3359_pins.hpp"

namespace HAL::MMCSD
{
    using REGS::MMCHS::AM335x_MMCHS_Type;
    using REGS::MMCHS::e_BUS_WIDTH;
    using REGS::MMCHS::e_CMD_INDEX;
    using REGS::MMCHS::e_DTO;
    using REGS::MMCHS::e_LINE_RESET;
    using REGS::MMCHS::e_RSP_TYPE;
    using REGS::MMCHS::e_SDVS;

    /*  Functional clock of the MMC/SD controllers on AM335x, driven by PER PLL M2.
     *  Used as the numerator of the SD_SYSCTL[CLKD] divider.
     */
    constexpr uint32_t INPUT_FREQ_HZ = 96'000'000u;

    /*  Bus frequencies of the card identification and data transfer phases. */
    constexpr uint32_t INIT_FREQ_HZ      =     400'000u;   // <= 400 kHz while the card is being identified
    constexpr uint32_t TRANSFER_FREQ_HZ  =  25'000'000u;   // default speed, up to 25 MHz
    constexpr uint32_t HIGH_SPEED_FREQ_HZ = 50'000'000u;   // high speed, up to 50 MHz

    /*  Polling budgets, in loop iterations: this layer is purely polled and owns no timer. */
    constexpr uint32_t RESET_RETRY_MAX       = 0xFFFFu;
    constexpr uint32_t RESET_START_RETRY_MAX = 0x3FFu;
    constexpr uint32_t POWER_RETRY_MAX       = 0xFFFFFu;
    constexpr uint32_t CLOCK_RETRY_MAX       = 0xFFFFu;
    constexpr uint32_t CMD_RETRY_MAX         = 0xFFFFFu;
    constexpr uint32_t XFER_RETRY_MAX        = 0xFFFFFFu;

    /*  Outcome of a command issued to the card. */
    enum class Status : uint8_t
    {
        OK = 0,
        NOT_INITIALIZED,        // the controller was never brought up
        BAD_ARGUMENT,
        TIMEOUT,                // the controller never raised command/transfer complete
        CMD_TIMEOUT,            // SD_STAT[16] CTO - the card did not answer
        CMD_ERROR,              // CRC / end bit / index error on the command response
        DATA_TIMEOUT,           // SD_STAT[20] DTO
        DATA_ERROR,             // CRC / end bit error during the data phase
        CARD_ERROR              // SD_STAT[28] CERR, the card reported an error in its response
    };

    /*  One command to push into SD_CMD, described in terms of the card protocol rather than
     *  of the raw register bits. mmcsd_base::send_command() turns it into the register word.
     */
    struct Command
    {
        e_CMD_INDEX index{REGS::MMCHS::CMD0_ACMD0};
        uint32_t    arg{0u};
        e_RSP_TYPE  rsp_type{REGS::MMCHS::RSP_NONE};
        bool        check_crc{true};        // enable the CRC7 check on the response
        bool        check_index{true};      // enable the index check on the response
        bool        data_present{false};    // the command carries a data phase on the DAT lines
        bool        read{true};             // data direction, only meaningful when data_present
        uint32_t    nblks{0u};              // number of blocks, only meaningful when data_present
        bool        auto_cmd12{false};      // let the controller close a multi block transfer itself
        bool        dma{false};             // set SD_CMD[0] DE, the caller owns the DMA programming

        /*  An ordinary command with no data phase, both response checks on. */
        [[nodiscard]] static constexpr Command simple(const e_CMD_INDEX idx,
                                                      const uint32_t arg,
                                                      const e_RSP_TYPE rsp) noexcept
        {
            Command c;
            c.index    = idx;
            c.arg      = arg;
            c.rsp_type = rsp;
            return c;
        }

        /*  A command whose response carries neither a CRC nor a command index, such as R3 (ACMD41).
         *  Leaving the checks on would make the controller flag every legal answer as an error.
         */
        [[nodiscard]] static constexpr Command unchecked(const e_CMD_INDEX idx,
                                                         const uint32_t arg,
                                                         const e_RSP_TYPE rsp) noexcept
        {
            Command c    = simple(idx, arg, rsp);
            c.check_crc  = false;
            c.check_index = false;
            return c;
        }

        /*  A command with a data phase on the DAT lines. Multi block transfers are closed by the
         *  controller itself through Auto CMD12.
         */
        [[nodiscard]] static constexpr Command transfer(const e_CMD_INDEX idx,
                                                        const uint32_t arg,
                                                        const bool is_read,
                                                        const uint32_t blocks) noexcept
        {
            Command c        = simple(idx, arg, REGS::MMCHS::RSP_48BIT);
            c.data_present   = true;
            c.read           = is_read;
            c.nblks          = blocks;
            c.auto_cmd12     = blocks > 1u;
            return c;
        }
    };

    /*  Bring-up parameters of the controller. */
    struct Config
    {
        e_SDVS       bus_voltage{REGS::MMCHS::VS_3_0V};
        e_BUS_WIDTH  bus_width{REGS::MMCHS::WIDTH_1BIT};
        e_DTO        data_timeout{REGS::MMCHS::DTO_TCF_2_27};
        uint32_t     bus_freq_hz{INIT_FREQ_HZ};
        bool         high_speed{false};
    };

    /*  Register context saved/restored around a power transition.
     *  SD_PSTATE is a read-only status register: it is captured for diagnostics only and never written back.
     */
    struct Context
    {
        uint32_t capa;
        uint32_t sysconfig;
        uint32_t con;
        uint32_t hctl;
        uint32_t sysctl;
        uint32_t pstate;
    };

    /*  Non-template half of the driver: everything that only needs the register block.
     *  It is deliberately free of any pin, clock or interrupt knowledge, so that the card protocol
     *  layer can drive any controller instance through a plain reference.
     */
    class mmcsd_base
    {
    protected:
        AM335x_MMCHS_Type& m_instance;

        explicit mmcsd_base(AM335x_MMCHS_Type* regs) noexcept
            : m_instance(*regs)
        {}

    public:
        [[nodiscard]] AM335x_MMCHS_Type& regs() const noexcept { return m_instance; }

        /// <--- Reset and system configuration ---> ///
        [[nodiscard]] bool soft_reset() noexcept;
        [[nodiscard]] bool lines_reset(uint32_t flag) noexcept;
        void system_config(uint32_t config) noexcept;

        /// <--- Card presence ---> ///
        [[nodiscard]] bool is_card_inserted() const noexcept;
        [[nodiscard]] bool is_card_write_protected() const noexcept;

        /// <--- Capabilities ---> ///
        [[nodiscard]] bool is_high_speed_supported() const noexcept;
        void supported_volt_set(uint32_t volt) noexcept;

        /// <--- Clocking ---> ///
        [[nodiscard]] bool is_int_clock_stable(uint32_t retry) const noexcept;
        [[nodiscard]] bool int_clock(bool on) noexcept;
        [[nodiscard]] bool bus_freq_set(uint32_t freq_in, uint32_t freq_out, bool bypass = false) noexcept;
        void card_clock_enable(bool on) noexcept;

        /// <--- Bus setup ---> ///
        void bus_width_set(e_BUS_WIDTH width) noexcept;
        void bus_volt_set(e_SDVS volt) noexcept;
        [[nodiscard]] bool bus_power(bool on) noexcept;
        void high_speed_enable(bool on) noexcept;
        void data_timeout_set(e_DTO timeout) noexcept;
        void blk_len_set(uint32_t blklen) noexcept;

        /// <--- Command path ---> ///
        [[nodiscard]] bool init_stream_send() noexcept;
        void command_send(const Command& cmd) noexcept;
        [[nodiscard]] Status send_command(const Command& cmd, uint32_t retry = CMD_RETRY_MAX) noexcept;
        void response_get(uint32_t* rsp) const noexcept;
        [[nodiscard]] uint32_t response_get_r1() const noexcept { return m_instance.RSP(0); }

        /// <--- Data path (PIO) ---> ///
        [[nodiscard]] Status data_read(uint8_t* dst, uint32_t len) noexcept;
        [[nodiscard]] Status data_write(const uint8_t* src, uint32_t len) noexcept;
        [[nodiscard]] Status wait_transfer_complete(uint32_t retry = XFER_RETRY_MAX) noexcept;

        /// <--- Status / interrupt management ---> ///
        [[nodiscard]] uint32_t intr_status_get(uint32_t flag) const noexcept;
        void intr_status_enable(uint32_t flag) noexcept;
        void intr_status_disable(uint32_t flag) noexcept;
        void intr_status_clear(uint32_t flag) noexcept;
        void intr_enable(uint32_t flag) noexcept;
        void intr_disable(uint32_t flag) noexcept;

        /// <--- Power management ---> ///
        void context_save(Context& ctx) const noexcept;
        void context_restore(const Context& ctx) noexcept;

        mmcsd_base(const mmcsd_base&) = delete;
        mmcsd_base& operator=(const mmcsd_base&) = delete;
        mmcsd_base(mmcsd_base&&) = delete;
        mmcsd_base& operator=(mmcsd_base&&) = delete;

    protected:
        ~mmcsd_base() = default;

        /*  Translate the error bits of SD_STAT into a Status and clear them. */
        [[nodiscard]] Status m_consume_errors() noexcept;
    };

    /*  CRTP shell that owns the bring-up of one concrete controller instance.
     *  The derived class supplies run_clocks() and init_pins(); everything else is shared.
     */
    template <typename Derived, uint32_t MMCBase, uint32_t IRQNum>
    class mmcsd : public mmcsd_base
    {
    protected:
        bool m_initialized{false};

        Derived& derived() noexcept { return static_cast<Derived&>(*this); }

    public:
        mmcsd() noexcept
            : mmcsd_base(reinterpret_cast<AM335x_MMCHS_Type*>(MMCBase))
        {}

        ~mmcsd() noexcept { deinit(); }

        [[nodiscard]] static constexpr uint32_t base_address() noexcept { return MMCBase; }
        [[nodiscard]] static constexpr uint32_t irq_number() noexcept { return IRQNum; }
        [[nodiscard]] bool is_initialized() const noexcept { return m_initialized; }

        /*  Bring the controller up to the point where the card identification sequence can start:
         *  clocks and pins on, controller reset, capabilities advertised, bus powered and clocked.
         */
        [[nodiscard]] bool init(const Config& cfg = {}) noexcept
        {
            using namespace REGS::MMCHS;

            deinit();

            derived().run_clocks();
            derived().init_pins();

            if (!soft_reset())
                return false;

            /* No idle acknowledge and free running clocks while the driver is polled. */
            system_config(static_cast<uint32_t>(SIDLE_NO) << 3);

            /* Advertise the voltages the board can actually supply before powering the bus. */
            supported_volt_set(CAPA_VOLT_3V0 | CAPA_VOLT_3V3);

            bus_volt_set(cfg.bus_voltage);

            if (!bus_power(true))
                return false;

            bus_width_set(cfg.bus_width);
            high_speed_enable(cfg.high_speed && is_high_speed_supported());
            data_timeout_set(cfg.data_timeout);

            if (!bus_freq_set(INPUT_FREQ_HZ, cfg.bus_freq_hz))
                return false;

            /* Reflect every event in SD_STAT, but leave the interrupt line masked: this driver polls. */
            intr_status_clear(EVENT_ALL_MASK);
            intr_status_enable(EVENT_ALL_MASK);
            intr_disable(EVENT_ALL_MASK);

            m_initialized = true;
            return true;
        }

        /*  Send the mandatory 80-clock initialization stream to the card. */
        [[nodiscard]] bool send_init_stream() noexcept
        {
            return m_initialized && init_stream_send();
        }

        void deinit() noexcept
        {
            /* A freshly constructed controller has no functional clock yet - do not touch its MMIO. */
            if (!m_initialized)
                return;

            intr_disable(REGS::MMCHS::EVENT_ALL_MASK);
            intr_status_disable(REGS::MMCHS::EVENT_ALL_MASK);
            card_clock_enable(false);
            (void)int_clock(false);
            (void)bus_power(false);

            m_initialized = false;
        }
    };

    /*  MMC0 of the BeagleBone Black: the microSD slot.
     *  Data/clock/command live on the dedicated MMC0 pads, card detect is muxed onto SPI0_CS1.
     */
    class mmcsd0_t : public mmcsd<mmcsd0_t,
                                  REGS::MMCHS::AM335x_MMCHS_0_BASE,
                                  REGS::INTC::MMCSD0INT>
    {
        using Base = mmcsd<mmcsd0_t,
                           REGS::MMCHS::AM335x_MMCHS_0_BASE,
                           REGS::INTC::MMCSD0INT>;
        friend Base;

        static void run_clocks() noexcept;
        static void init_pins() noexcept;

    public:
        mmcsd0_t() noexcept = default;
    };

}   // namespace HAL::MMCSD

#endif //HAL_MMCSD_MMCSD_HPP
