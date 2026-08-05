#ifndef HAL_PARAMBUILDER_HPP
#define HAL_PARAMBUILDER_HPP

#include "regs/EDMA.hpp"

namespace HAL::EDMA
{
    class ParamBuilder
    {
    private:
        REGS::EDMA::paRAM_entry_t entry{};

    public:
        ParamBuilder() noexcept
        {
            entry.OPT.reg = 0;
            entry.LINK = 0xFFFF; // No link
        }

        constexpr ParamBuilder& setSource(uintptr_t src_addr, int16_t bidx = 0, int16_t cidx = 0) noexcept
        {
            entry.SRC = static_cast<uint32_t>(src_addr);
            entry.SRCBIDX = bidx;
            entry.SRCCIDX = cidx;
            return *this;
        }

        constexpr ParamBuilder& setDest(uintptr_t dst_addr, int16_t bidx = 0, int16_t cidx = 0) noexcept
        {
            entry.DST = static_cast<uint32_t>(dst_addr);
            entry.DSTBIDX = bidx;
            entry.DSTCIDX = cidx;
            return *this;
        }

        constexpr ParamBuilder& setTransferParams(uint16_t acnt, uint16_t bcnt = 1, uint16_t ccnt = 1) noexcept
        {
            entry.ACNT = acnt;
            entry.BCNT = bcnt;
            entry.CCNT = ccnt;
            return *this;
        }

        constexpr ParamBuilder& enableCompletionInterrupt(uint8_t tcc_channel) noexcept
        {
            entry.OPT.b.TCC = tcc_channel;
            entry.OPT.b.TCINTEN = 1; // Transfer Complete Interrupt Enable
            return *this;
        }

        constexpr ParamBuilder& setSyncType(const bool is_ab_sync) noexcept
        {
            entry.OPT.b.SYNCDIM = is_ab_sync ? 1 : 0; // 0 = A-Sync, 1 = AB-Sync
            return *this;
        }

        [[nodiscard]] REGS::EDMA::paRAM_entry_t build() const noexcept {
            return entry;
        }
    };
}

#endif //HAL_PARAMBUILDER_HPP