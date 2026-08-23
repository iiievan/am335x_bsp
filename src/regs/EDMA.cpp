#include "regs/EDMA.hpp"

namespace REGS::EDMA
{
    paRAM_entry_t& AM335x_EDMA3CC_Type::paRAM(const uint32_t n) const noexcept
    {
        return *reinterpret_cast<paRAM_entry_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n));
    }

    uint32_t& AM335x_EDMA3CC_Type::OPT(const uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0x0);
    }

    uint32_t& AM335x_EDMA3CC_Type::SRC(uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0x4);
    }

    uint32_t& AM335x_EDMA3CC_Type::ACNT_BCNT(uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0x8);
    }

    uint32_t& AM335x_EDMA3CC_Type::DST(uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0xC);
    }

    uint32_t& AM335x_EDMA3CC_Type::SRC_DST_BIDX(uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0x10);
    }

    uint32_t& AM335x_EDMA3CC_Type::LINK_BCNTRLD(uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0x14);
    }

    uint32_t& AM335x_EDMA3CC_Type::SRC_DST_CIDX(uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0x18);
    }

    uint32_t& AM335x_EDMA3CC_Type::CCNT(uint32_t n) const noexcept
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(this) + PARAM_BASE + (0x20 * n) + 0x1C);
    }

    DRAE_reg_t& AM335x_EDMA3CC_Type::DRAE(const e_REGION_ID region_id) const noexcept
    {
        return *(reinterpret_cast<DRAE_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x340 + (static_cast<uint32_t>(region_id) * 8)));
    }

    DRAEH_reg_t& AM335x_EDMA3CC_Type::DRAEH(const e_REGION_ID region_id) const noexcept
    {
        return *(reinterpret_cast<DRAEH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x344 + (static_cast<uint32_t>(region_id) * 8)));
    }

    ER_reg_t& AM335x_EDMA3CC_Type::S_ER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2000 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    ERH_reg_t & AM335x_EDMA3CC_Type::S_ERH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ERH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2004 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    ECR_reg_t& AM335x_EDMA3CC_Type::S_ECR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ECR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2008 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    ECRH_reg_t& AM335x_EDMA3CC_Type::S_ECRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ECRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x200C + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    ESR_reg_t& AM335x_EDMA3CC_Type::S_ESR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ESR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2010 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    ESRH_reg_t& AM335x_EDMA3CC_Type::S_ESRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ESRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2014 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    CER_reg_t& AM335x_EDMA3CC_Type::S_CER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<CER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2018 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    CERH_reg_t & AM335x_EDMA3CC_Type::S_CERH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<CERH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x201C + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    EER_reg_t& AM335x_EDMA3CC_Type::S_EER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<EER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2020 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    EERH_reg_t& AM335x_EDMA3CC_Type::S_EERH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<EERH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2024 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    EECR_reg_t& AM335x_EDMA3CC_Type::S_EECR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<EECR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2028 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    EECRH_reg_t& AM335x_EDMA3CC_Type::S_EECRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<EECRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x202C + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    EESR_reg_t& AM335x_EDMA3CC_Type::S_EESR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<EESR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2030 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    EESRH_reg_t& AM335x_EDMA3CC_Type::S_EESRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<EESRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2034 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    SER_reg_t& AM335x_EDMA3CC_Type::S_SER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<SER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2038 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    SERH_reg_t& AM335x_EDMA3CC_Type::S_SERH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<SERH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x203C + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    SECR_reg_t& AM335x_EDMA3CC_Type::S_SECR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<SECR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2040 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    SECRH_reg_t& AM335x_EDMA3CC_Type::S_SECRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<SECRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2044 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IER_reg_t& AM335x_EDMA3CC_Type::S_IER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2050 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IERH_reg_t& AM335x_EDMA3CC_Type::S_IERH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IERH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2054 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IECR_reg_t& AM335x_EDMA3CC_Type::S_IECR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IECR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2058 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IECRH_reg_t& AM335x_EDMA3CC_Type::S_IECRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IECRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x205C + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IESR_reg_t& AM335x_EDMA3CC_Type::S_IESR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IESR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2060 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IESRH_reg_t& AM335x_EDMA3CC_Type::S_IESRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IESRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2064 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IPR_reg_t& AM335x_EDMA3CC_Type::S_IPR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IPR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2068 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IPRH_reg_t& AM335x_EDMA3CC_Type::S_IPRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IPRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x206C + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    ICR_reg_t& AM335x_EDMA3CC_Type::S_ICR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ICR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2070 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    ICRH_reg_t& AM335x_EDMA3CC_Type::S_ICRH(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<ICRH_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2074 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    IEVAL_reg_t& AM335x_EDMA3CC_Type::S_IEVAL(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<IEVAL_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2078 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    QER_reg_t& AM335x_EDMA3CC_Type::S_QER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<QER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2080 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    QEER_reg_t& AM335x_EDMA3CC_Type::S_QEER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<QEER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2084 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    QEECR_reg_t& AM335x_EDMA3CC_Type::S_QEECR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<QEECR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2088 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    QEESR_reg_t& AM335x_EDMA3CC_Type::S_QEESR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<QEESR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x208C + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    QSER_reg_t& AM335x_EDMA3CC_Type::S_QSER(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<QSER_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2090 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    QSECR_reg_t& AM335x_EDMA3CC_Type::S_QSECR(const e_REGION_ID region_id) const noexcept
    {
        return *reinterpret_cast<QSECR_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x2094 + (static_cast<uint32_t>(region_id) * SHADOW_STRIDE));
    }

    __RW DMAQNUM_reg_t & AM335x_EDMA3CC_Type::get_DMAQNUM_idx(uint32_t channel) noexcept
    {
        return this->DMAQNUM[channel >> 3];
    }

    // ========== AM335x_EDMA3TC_Type ==========

    DFOPT_reg_t& AM335x_EDMA3TC_Type::OPT(uint32_t n) const noexcept
    {
#ifdef am335x
         n = 0;
#endif
        return *reinterpret_cast<DFOPT_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x300 + (n * 0x40));
    }

    DFSRC_reg_t& AM335x_EDMA3TC_Type::SRC(uint32_t n) const noexcept
    {
#ifdef am335x
        n = 0;
#endif
        return *reinterpret_cast<DFSRC_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x304 + (n * 0x40));
    }

    DFCNT_reg_t& AM335x_EDMA3TC_Type::CNT(uint32_t n) const noexcept
    {
#ifdef am335x
        n = 0;
#endif
        return *reinterpret_cast<DFCNT_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x308 + (n * 0x40));
    }

    DFDST_reg_t& AM335x_EDMA3TC_Type::DST(uint32_t n) const noexcept
    {
#ifdef am335x
        n = 0;
#endif
        return *reinterpret_cast<DFDST_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x30C + (n * 0x40));
    }

    DFBIDX_reg_t& AM335x_EDMA3TC_Type::BIDX(uint32_t n) const noexcept
    {
#ifdef am335x
        n = 0;
#endif
        return *reinterpret_cast<DFBIDX_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x310 + (n * 0x40));
    }

    DFMPPRXY_reg_t& AM335x_EDMA3TC_Type::MPPRXY(uint32_t n) const noexcept
    {
#ifdef am335x
        n = 0;
#endif
        return *reinterpret_cast<DFMPPRXY_reg_t*>(
            reinterpret_cast<uintptr_t>(this) + 0x314 + (n * 0x40));
    }
}

