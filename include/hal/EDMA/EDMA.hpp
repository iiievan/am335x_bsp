#ifndef __HAL_EDMA_HPP
#define __HAL_EDMA_HPP

#include <cstdint>
#include "../../regs/REGS.hpp"

namespace HAL::EDMA
{
        void  module_clock_config() noexcept;
        void  init(REGS::EDMA::e_EVENT_QUEUE que_num) noexcept;
        void  set_non_idle_mode() noexcept;
        void  enable_ch_in_shadow_reg(REGS::EDMA::e_EDMA3_CH_TYPE ch_type, uint32_t ch_num) noexcept;
        void  disable_ch_in_shadow_reg(REGS::EDMA::e_EDMA3_CH_TYPE ch_type, uint32_t ch_num) noexcept;
        void  map_ch_to_evtQ(REGS::EDMA::e_EDMA3_CH_TYPE ch_type, uint32_t ch_num, REGS::EDMA::e_EVENT_QUEUE evt_Qnum) noexcept;
        void  unmap_ch_to_evtQ(REGS::EDMA::e_EDMA3_CH_TYPE ch_type, uint32_t ch_num) noexcept;
        void  map_QDMA_ch_to_paRAM(uint32_t ch_num, uint32_t *paRAM_id) noexcept;
        void  set_QDMA_trig_word(uint32_t ch_num, uint8_t trig_word) noexcept;
        void  clr_miss_evt(uint32_t ch_num) noexcept;
        void  QDMA_clr_miss_evt(uint32_t ch_num) noexcept;
        void  clr_CC_Err(uint32_t flags) noexcept;
        void  set_event(uint32_t ch_num) noexcept;
        void  clr_event(uint32_t ch_num) noexcept;
        void  enable_DMA_event(uint32_t ch_num) noexcept;
        void  disable_DMA_event(uint32_t ch_num) noexcept;
        void  enable_QDMA_event(uint32_t ch_num) noexcept;
        void  disable_QDMA_event(uint32_t ch_num) noexcept;
    uint32_t  get_intr_status() noexcept;
        void  enable_evt_intr(uint32_t ch_num) noexcept;
        void  disable_evt_intr(uint32_t ch_num) noexcept;
        void  clr_intr(uint32_t value) noexcept;
        void  get_paRAM(uint32_t paRAM_id, REGS::EDMA::paRAM_entry_t& dst) noexcept;
        [[nodiscard]] REGS::EDMA::paRAM_entry_t get_paRAM(uint32_t paRAM_id) noexcept;
        void  QDMA_get_paRAM(uint32_t paRAM_id, REGS::EDMA::paRAM_entry_t& dst) noexcept;
        [[nodiscard]] REGS::EDMA::paRAM_entry_t QDMA_get_paRAM(uint32_t paRAM_id) noexcept;
        void  set_paRAM(uint32_t ch_num, const REGS::EDMA::paRAM_entry_t& src) noexcept;
        void  set_paRAM(uint32_t ch_num, const REGS::EDMA::paRAM_entry_t* src) noexcept;
        void  QDMA_set_paRAM( uint32_t ch_num, const REGS::EDMA::paRAM_entry_t& src) noexcept;
        void  QDMA_set_paRAM(uint32_t paRAM_id, const REGS::EDMA::paRAM_entry_t* new_paRAM) noexcept;
        void  QDMA_set_paRAM_entry(uint32_t paRAM_id, uint32_t paRAM_entry, uint32_t new_paRAM_entry_val) noexcept;
    uint32_t  QDMA_get_paRAM_entry(uint32_t paRAM_id, uint32_t paRAM_entry) noexcept;
        bool  request_channel(REGS::EDMA::e_EDMA3_CH_TYPE ch_type, uint32_t ch_num, uint32_t tcc_num, REGS::EDMA::e_EVENT_QUEUE evt_Qnum) noexcept;
        bool  free_channel(REGS::EDMA::e_EDMA3_CH_TYPE ch_type, uint32_t ch_num, uint32_t trig_mode, uint32_t tcc_num, REGS::EDMA::e_EVENT_QUEUE evt_Qnum) noexcept;
        bool  enable_transfer(uint32_t ch_num, uint32_t trig_mode) noexcept;
        bool  disable_transfer(uint32_t ch_num, uint32_t trig_mode) noexcept;
        void  clear_error_bits(uint32_t ch_num, REGS::EDMA::e_EVENT_QUEUE evt_Qnum) noexcept;
    uint32_t  get_CC_Err_status() noexcept;
    uint32_t  get_Err_intr_status() noexcept;
    uint32_t  QDMA_get_Err_intr_status() noexcept;
        void  CC_Err_evaluate() noexcept;
        void  deinit(uint32_t que_num) noexcept;
    inline uint32_t  version_get() noexcept  { return 1; } // This returns a number '2' which is unique to EDMA IP in AM335x.
    uint32_t  peripheral_id_get() noexcept;
    uint32_t  intr_status_high_get() noexcept;
    uint32_t  Err_intr_high_status_get() noexcept;
        void  channel_to_param_map(uint32_t channel, uint32_t param_set) noexcept;
        void  context_save(REGS::EDMA::EDMACONTEXT_t *p_edma_cntx) noexcept;
        void  context_restore(const REGS::EDMA::EDMACONTEXT_t *p_edma_cntx) noexcept;

}

#endif //__HAL_EDMA_HPP