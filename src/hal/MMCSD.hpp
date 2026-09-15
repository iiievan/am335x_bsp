#ifndef HAL_MMCSD_HPP
#define HAL_MMCSD_HPP

#include <cstdint>

#include "regs/MMCHS.hpp"

namespace HAL::MMCSD
{
    using REGS::MMCHS::AM335x_MMCHS_Type;
    using REGS::MMCHS::e_BUS_WIDTH;
    using REGS::MMCHS::e_DTO;
    using REGS::MMCHS::e_LINE_RESET;
    using REGS::MMCHS::e_SDVS;

    /*  Register context saved/restored around a power transition.
     *  SD_PSTATE is a read-only status register: it is captured for diagnostics only and is never written back.
     */
    struct MMCSDContext_t
    {
        uint32_t capa;
        uint32_t systemConfig;
        uint32_t ctrlInfo;
        uint32_t hctl;
        uint32_t sysCtl;
        uint32_t pState;
    };

    /*  Default polling budgets, expressed in loop iterations (this layer is purely polled, it owns no timer). */
    constexpr uint32_t RESET_RETRY_MAX = 0xFFFFu;
    constexpr uint32_t RESET_START_RETRY_MAX = 0x3FFu;   // short window used to catch the start of a line reset
    constexpr uint32_t POWER_RETRY_MAX = 0xFFFFFu;
    constexpr uint32_t CLOCK_RETRY_MAX = 0xFFFFu;
    constexpr uint32_t CMD_RETRY_MAX   = 0xFFFFu;

    void                 construct_mmcsd(void * base_addr) noexcept;
    bool                 is_constructed() noexcept;
    AM335x_MMCHS_Type *  instance() noexcept;

    int       soft_reset() noexcept;
    int       lines_reset(uint32_t flag) noexcept;
    void      system_config(uint32_t config) noexcept;

    uint32_t  is_card_inserted() noexcept;
    uint32_t  is_card_write_protected() noexcept;
    uint32_t  is_hsupported() noexcept;

    void      supported_volt_set(uint32_t volt) noexcept;
    uint32_t  is_int_clock_stable(uint32_t retry) noexcept;
    int       int_clock(uint32_t pwr) noexcept;

    int       bus_freq_set(uint32_t freq_in, uint32_t freq_out, uint32_t bypass) noexcept;
    void      bus_width_set(uint32_t width) noexcept;
    int       bus_power(uint32_t pwr) noexcept;
    void      bus_volt_set(uint32_t volt) noexcept;

    void      command_send(uint32_t cmd, uint32_t cmdarg, const void * data, uint32_t nblks, uint32_t dmaEn) noexcept;
    void      data_get(uint8_t * data, uint32_t len) noexcept;
    uint32_t  is_xfer_complete(uint32_t retry) noexcept;
    uint32_t  is_cmd_complete(uint32_t retry) noexcept;
    void      response_get(uint32_t * rsp) noexcept;
    void      blk_len_set(uint32_t blklen) noexcept;
    int       init_stream_send() noexcept;
    void      data_timeout_set(uint32_t timeout) noexcept;

    uint32_t  intr_status_get(uint32_t flag) noexcept;
    void      intr_enable(uint32_t flag) noexcept;
    void      intr_status_enable(uint32_t flag) noexcept;
    void      intr_status_disable(uint32_t flag) noexcept;
    void      intr_status_clear(uint32_t flag) noexcept;

    void      context_save(MMCSDContext_t * contextPtr) noexcept;
    void      context_restore(const MMCSDContext_t * contextPtr) noexcept;

}   // namespace HAL::MMCSD

#endif //HAL_MMCSD_HPP
