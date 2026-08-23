#ifndef HAL_PARAMBUILDER_HPP
#define HAL_PARAMBUILDER_HPP

#include "regs/EDMA.hpp"
#include <cstdint>

namespace HAL::EDMA
{
    /**
     * @brief Fluent builder for EDMA3 PaRAM entries (AM335x).
     *
     * Low-level setters + high-level presets for common transfer patterns.
     */
    class ParamBuilder
    {
    private:
        REGS::EDMA::paRAM_entry_t entry{};

    public:
        ParamBuilder() noexcept
        {
            entry.OPT.reg = 0u;

            entry.SRC = 0u;
            entry.ACNT = 0u;
            entry.BCNT = 0u;
            entry.DST = 0u;
            entry.SRCBIDX = 0;
            entry.DSTBIDX = 0;
            entry.LINK = 0xFFFFu;
            entry.BCNTRLD = 0u;
            entry.SRCCIDX = 0;
            entry.DSTCIDX = 0;
            entry.CCNT = 0u;
        }

        // -----------------------------------------------------------------
        // Low-level setters (original API, kept for full control)
        // -----------------------------------------------------------------

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

        constexpr ParamBuilder& setLink(uint16_t link_addr) noexcept
        {
            entry.LINK = link_addr;   // address of next PaRAM (usually param_id * 0x20)
            return *this;
        }

        constexpr ParamBuilder& setTransferParams(uint16_t acnt, uint16_t bcnt = 1, uint16_t ccnt = 1) noexcept
        {
            entry.ACNT = acnt;
            entry.BCNT = bcnt;
            entry.CCNT = ccnt;
            return *this;
        }

        constexpr ParamBuilder& enableCompletionInterrupt(uint8_t tcc_channel, const bool value = true) noexcept
        {
            entry.OPT.b.TCC = tcc_channel;
            entry.OPT.b.TCINTEN = value ? 1u : 0u;
            return *this;
        }

        constexpr ParamBuilder& enableIntermediateCompletionInterrupt(const bool value = true) noexcept
        {
            entry.OPT.b.ITCINTEN = value ? 1u : 0u;
            return *this;
        }

        constexpr ParamBuilder& enableTransferCompleteChaining(uint8_t tcc, const bool value = true) noexcept
        {
            entry.OPT.b.TCC    = tcc;
            entry.OPT.b.TCCHEN = value ? 1u : 0u;
            return *this;
        }

        constexpr ParamBuilder& enableIntermediateTransferCompleteChaining(const bool value = true) noexcept
        {
            entry.OPT.b.ITCCHEN = value ? 1u : 0u;
            return *this;
        }

        constexpr ParamBuilder& setFIFOWidth(REGS::EDMA::e_paRAM_FIFO_WIDTH fwid) noexcept
        {
            if (fwid > REGS::EDMA::FIFO_WIDTH_256BIT)
                entry.OPT.b.FWID = REGS::EDMA::FIFO_WIDTH_8BIT;
            else
                entry.OPT.b.FWID = fwid;
            return *this;
        }

        constexpr ParamBuilder& setStatic(const bool value = true) noexcept
        {
            entry.OPT.b.STATIC = value ? 1u : 0u;
            return *this;
        }

        constexpr ParamBuilder& setSrcDstDestinationMode(const bool DAM = false, const bool SAM = false) noexcept
        {
            entry.OPT.b.DAM = DAM ? 1u : 0u;
            entry.OPT.b.SAM = SAM ? 1u : 0u;
            return *this;
        }

        constexpr ParamBuilder& setSyncType(const bool is_ab_sync) noexcept
        {
            entry.OPT.b.SYNCDIM = is_ab_sync ? 1u : 0u; // 0 = A-Sync, 1 = AB-Sync
            return *this;
        }

        // -----------------------------------------------------------------
        // High-level presets (reduce boilerplate)
        // -----------------------------------------------------------------

        /**
         * @brief Classic A-synchronized transfer (1-D).
         * BIDX is set to @p size so address advances after each A-transfer
         * when BCNT > 1 (though for pure A-Sync BCNT is usually 1).
         */
        constexpr ParamBuilder& setASyncTransfer(uintptr_t src, uintptr_t dst,
                                                 uint16_t size,
                                                 bool increment = true) noexcept
        {
            const int16_t bidx = increment ? static_cast<int16_t>(size) : 0;
            return setSource(src, bidx, 0)
                  .setDest(dst, bidx, 0)
                  .setTransferParams(size, 1, 1)
                  .setSyncType(false);
        }

        /**
         * @brief AB-synchronized transfer (2-D).
         * BIDX = ACNT so rows are contiguous.
         */
        constexpr ParamBuilder& setABSyncTransfer(uintptr_t src, uintptr_t dst,
                                                  uint16_t acnt, uint16_t bcnt) noexcept
        {
            return setSource(src, static_cast<int16_t>(acnt), 0)
                  .setDest(dst, static_cast<int16_t>(acnt), 0)
                  .setTransferParams(acnt, bcnt, 1)
                  .setSyncType(true);
        }

        /**
         * @brief Point LINK to another PaRAM set (param_id * 0x20) and clear STATIC.
         * Required for linking / chaining / self-link.
         */
        constexpr ParamBuilder& linkTo(uint8_t next_param_id) noexcept
        {
            return setStatic(false)
                  .setLink(static_cast<uint16_t>(next_param_id * 0x20));
        }

        /**
         * @brief Self-link: reload the same PaRAM after completion.
         */
        constexpr ParamBuilder& setSelfLink(uint8_t param_id) noexcept
        {
            return linkTo(param_id);
        }

        /**
         * @brief Mark as final entry of a chain (STATIC=1, LINK=0xFFFF).
         */
        constexpr ParamBuilder& endOfChain() noexcept
        {
            return setStatic(true)
                  .setLink(0xFFFF);
        }

        [[nodiscard]] REGS::EDMA::paRAM_entry_t build() const noexcept
        {
            return entry;
        }
    };

    // =====================================================================
    // Factory of ready-to-use PaRAM configurations
    // =====================================================================

    /**
     * @brief Static factory for the most common EDMA3 transfer patterns
     * used in the test suite (A, AB, Chain, Ping-Pong, Self-link).
     *
     * All addresses are byte addresses; sizes are in bytes.
     * TCC is the Transfer Complete Code (usually equal to the channel number).
     */
    class PaRAMFactory
    {
    public:
        // -----------------------------------------------------------------
        // Simple single transfers
        // -----------------------------------------------------------------

        /**
         * @brief Legal zero-byte transfer used to absorb a peripheral event.
         *
         * At least one count is zero and at least one count is nonzero.
         * The request is serviced without setting EMR/EMRH/QEMR.
         *
         * Intended primarily as the terminal linked PaRAM for UART TX.
         */
        static REGS::EDMA::paRAM_entry_t makeDummy(const uint8_t tcc)
        {
            return ParamBuilder()
                .setSource(0u, 0, 0)
                .setDest(0u, 0, 0)
                .setTransferParams(0u, 1u, 1u)
                .setSyncType(false)
                .enableCompletionInterrupt(tcc, false)
                .enableIntermediateCompletionInterrupt(false)
                .enableTransferCompleteChaining(tcc, false)
                .enableIntermediateTransferCompleteChaining(false)
                .endOfChain()
                .build();
        }

        /**
         * @brief Null PaRAM used for Event Missed error-path testing.
         *
         * Triggering this PaRAM is intentionally an error condition:
         * EMR/EMRH/QEMR is set and the secondary event remains latched.
         *
         * !!!Do not use as an idle or terminal UART PaRAM!!!
         */
        static REGS::EDMA::paRAM_entry_t makeNullforTest(const uint8_t tcc)
        {
            return ParamBuilder()
                .setSource(0u, 0, 0)
                .setDest(0u, 0, 0)
                .setTransferParams(0u, 0u, 0u)
                .setSyncType(false)
                .enableCompletionInterrupt(tcc, false)
                .enableIntermediateCompletionInterrupt(false)
                .enableTransferCompleteChaining(tcc, false)
                .enableIntermediateTransferCompleteChaining(false)
                .endOfChain()
                .build();
        }

        /**
         * @brief A-Sync transfer (most common case).
         * @param is_static  true for one-shot / QDMA, false when linking is needed
         */
        static REGS::EDMA::paRAM_entry_t makeASync(const uintptr_t src,
                                                   const uintptr_t dst,
                                                   const uint16_t size,
                                                   const uint8_t tcc,
                                                   const bool enable_irq = true,
                                                   const bool is_static  = true,
                                                   const bool increment  = true)
        {
            return ParamBuilder()
                .setASyncTransfer(src, dst, size, increment)
                .enableCompletionInterrupt(tcc, enable_irq)
                .setStatic(is_static)
                .build();
        }

        /**
         * @brief AB-Sync transfer.
         */
        static REGS::EDMA::paRAM_entry_t makeABSync(const uintptr_t src,
                                                    const uintptr_t dst,
                                                    const uint16_t acnt,
                                                    const uint16_t bcnt,
                                                    const uint8_t tcc,
                                                    const bool enable_irq = true,
                                                    const bool is_static  = true)
        {
            return ParamBuilder()
                .setABSyncTransfer(src, dst, acnt, bcnt)
                .enableCompletionInterrupt(tcc, enable_irq)
                .setStatic(is_static)
                .build();
        }

        // -----------------------------------------------------------------
        // Chain (two PaRAM sets)
        // -----------------------------------------------------------------

        struct ChainPair
        {
            REGS::EDMA::paRAM_entry_t first; // STATIC=0, TCCHEN, LINK → last
            REGS::EDMA::paRAM_entry_t last;  // STATIC=1, TCINTEN, LINK=0xFFFF
        };

        /**
         * @brief Two-element chain that copies [src .. src+2*half) → [dst .. dst+2*half).
         * First half uses chaining (TCCHEN), second half raises the completion interrupt.
         */
        static ChainPair makeChain(const uintptr_t src,
                                   const uintptr_t dst,
                                   const uint16_t half_size,
                                   const uint8_t param0,
                                   const uint8_t param1,
                                   const uint8_t tcc)
        {
            (void)param0;
            const auto last = ParamBuilder()
                .setSource(src + half_size, static_cast<int16_t>(half_size), 0)
                .setDest  (dst + half_size, static_cast<int16_t>(half_size), 0)
                .setTransferParams(half_size, 1, 1)
                .setSyncType(false)                         // <- Async
                .enableCompletionInterrupt(tcc, true)
                .endOfChain()
                .build();

            const auto first = ParamBuilder()
                .setSource(src, static_cast<int16_t>(half_size), 0)
                .setDest  (dst, static_cast<int16_t>(half_size), 0)
                .setTransferParams(half_size, 1, 1)
                .setSyncType(false)                         // <- Async
                .enableTransferCompleteChaining(tcc, true) // no interrupt on first
                .linkTo(param1)
                .build();

            return {first, last};
        }

        // -----------------------------------------------------------------
        // Ping-Pong
        // -----------------------------------------------------------------

        struct PingPongPair
        {
            REGS::EDMA::paRAM_entry_t ping; // LINK → pong
            REGS::EDMA::paRAM_entry_t pong; // LINK → ping
        };

        struct QdmaLinkPair
        {
            REGS::EDMA::paRAM_entry_t first;    // STATIC=0, TCINTEN=0, TCCHEN=0, LINK → last
            REGS::EDMA::paRAM_entry_t last;     // STATIC=1, TCINTEN=1, TCCHEN=0, LINK=0xFFFF
        };

        /**
         * @brief Classic hardware ping-pong.
         * Both sets raise TCC interrupt and point to each other (STATIC=0).
         */
        static PingPongPair makePingPong(const uintptr_t src,
                                         const uintptr_t dst_a,
                                         const uintptr_t dst_b,
                                         const uint16_t half,
                                         const uint8_t ping_id,
                                         const uint8_t pong_id,
                                         const uint8_t tcc)
        {
            const auto pong = ParamBuilder()
                .setSource(src + half, static_cast<int16_t>(half), 0)
                .setDest  (dst_b,      static_cast<int16_t>(half), 0)
                .setTransferParams(half, 1, 1)
                .setSyncType(false)
                .enableCompletionInterrupt(tcc, true)
                .linkTo(ping_id)
                .build();

            const auto ping = ParamBuilder()
                .setSource(src,   static_cast<int16_t>(half), 0)
                .setDest  (dst_a, static_cast<int16_t>(half), 0)
                .setTransferParams(half, 1, 1)
                .setSyncType(false)
                .enableCompletionInterrupt(tcc, true)
                .linkTo(pong_id)
                .build();

            return {ping, pong};
        }

        // -----------------------------------------------------------------
        // Self-link
        // -----------------------------------------------------------------

        /**
         * @brief Single PaRAM that reloads itself after every completion.
         * Useful for continuous / multi-shot transfers driven by manual or event triggers.
         */
        static REGS::EDMA::paRAM_entry_t makeSelfLink(const uintptr_t src,
                                                      const uintptr_t dst,
                                                      const uint16_t size,
                                                      const uint8_t param_id,
                                                      const uint8_t tcc,
                                                      const bool enable_irq = true)
        {
            return ParamBuilder()
                .setASyncTransfer(src, dst, size)
                .enableCompletionInterrupt(tcc, enable_irq)
                .setSelfLink(param_id)
                .build();
        }

        // -----------------------------------------------------------------
        // QDMA helpers (usually STATIC=1)
        // -----------------------------------------------------------------

        /**
         * @brief A-Sync ready for QDMA (STATIC forced to true).
         */
        static REGS::EDMA::paRAM_entry_t makeQdmaASync(const uintptr_t src,
                                                       const uintptr_t dst,
                                                       const uint16_t size,
                                                       const uint8_t tcc,
                                                       const bool enable_irq = true)
        {
            return makeASync(src, dst, size, tcc, enable_irq, /*is_static=*/true);
        }

        /**
         * @brief AB-Sync ready for QDMA (STATIC forced to true).
         */
        static REGS::EDMA::paRAM_entry_t makeQdmaABSync(const uintptr_t src,
                                                        const uintptr_t dst,
                                                        const uint16_t acnt,
                                                        const uint16_t bcnt,
                                                        const uint8_t tcc,
                                                        const bool enable_irq = true)
        {
            return makeABSync(src, dst, acnt, bcnt, tcc, enable_irq, /*is_static=*/true);
        }

        /**
        * @brief Two-element QDMA linked transfer.
        *
        * The first transfer performs no completion chaining. Its link reload
        * writes the second PaRAM into the QCHMAP-mapped PaRAM set, thereby
        * producing the next QDMA event when the selected trigger word is written.
        * The second transfer generates the final completion interrupt.
        */
        static QdmaLinkPair makeQdmaLink(const uintptr_t src,
                                          const uintptr_t dst,
                                          const uint16_t half_size,
                                          const uint8_t next_param,
                                          const uint8_t tcc)
        {
            const auto last = ParamBuilder()
                .setSource(src + half_size,static_cast<int16_t>(half_size),0)
                .setDest(dst + half_size, static_cast<int16_t>(half_size),0)
                .setTransferParams(half_size, 1, 1)
                .setSyncType(false)
                .enableCompletionInterrupt(tcc, true)
                .endOfChain()
                .build();

            const auto first = ParamBuilder()
                .setSource(src,static_cast<int16_t>(half_size),0)
                .setDest(dst,static_cast<int16_t>(half_size),0)
                .setTransferParams(half_size, 1, 1)
                .setSyncType(false)
                // Сохраняем правильный TCC для диагностики и Error ISR,
                // но прерывание после первого элемента не создаём.
                .enableCompletionInterrupt(tcc, false)
                .linkTo(next_param)// TCCHEN остаётся равным нулю.
                .build();

            return {first, last};
        }
    };
} // namespace HAL::EDMA

#endif // HAL_PARAMBUILDER_HPP