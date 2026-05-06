/*=======================================================================*/
/*  hal_mmu_cache.cpp - MMU & Cache implementation                      */
/*=======================================================================*/
#include "hal/CACHE.hpp"
#include "startup/cp15.h"

namespace HAL::CACHE
{
    static constexpr uint32_t CORTEX_A8_L2EN = 0x02;
    static constexpr uint32_t PRIMARY_PART_CORTEX_A8 = 0xC08;

    static bool is_cortex_a8() noexcept { return cp15_main_id_prim_part_num_get() == PRIMARY_PART_CORTEX_A8; }

    void init() noexcept
    {
        // Clean & invalidate D-Cache, invalidate I-Cache before enabling
        dcache_clean_invalidate_all();
        icache_invalidate_all();
    }

    void enable(Type cache_type) noexcept
    {
        auto flag = static_cast<uint32_t>(cache_type);

        if (flag & static_cast<uint32_t>(Type::ICACHE))
        {
            cp15_I_cache_flush();
            cp15_I_cache_enable();
        }

        if (flag & static_cast<uint32_t>(Type::DCACHE))
        {
            // For Cortex-A8, enable L2 cache first
            if (is_cortex_a8())
                cp15_aux_control_feature_enable(CORTEX_A8_L2EN);

            cp15_D_cache_flush();
            cp15_D_cache_enable();
        }

        cp15_DSB_ISB_sync_barrier();
    }

    void disable(Type cache_type) noexcept
    {
        auto flag = static_cast<uint32_t>(cache_type);

        if (flag & static_cast<uint32_t>(Type::ICACHE))
            cp15_I_cache_disable();

        if (flag & static_cast<uint32_t>(Type::DCACHE))
        {
            cp15_D_cache_disable();

            if (is_cortex_a8())
                cp15_aux_control_feature_disable(CORTEX_A8_L2EN);
        }

        cp15_DSB_ISB_sync_barrier();
    }

    void icache_invalidate_all() noexcept { cp15_I_cache_flush(); }
    void icache_invalidate_range(uint32_t start_addr, uint32_t size) noexcept { cp15_I_cache_flush_buff(start_addr, size); }
    void dcache_clean_all() noexcept { cp15_D_cache_clean(); }
    void dcache_invalidate_all() noexcept { cp15_D_cache_flush(); }
    void dcache_clean_invalidate_all() noexcept { cp15_D_cache_clean_flush(); }

    void dcache_clean_range(uint32_t start_addr, uint32_t size) noexcept { cp15_D_cache_clean_buff(start_addr, size); }
    void dcache_invalidate_range(uint32_t start_addr, uint32_t size) noexcept { cp15_D_cache_flush_buff(start_addr, size); }
    void dcache_clean_invalidate_range(uint32_t start_addr, uint32_t size) noexcept { cp15_D_cache_clean_flush_buff(start_addr, size); }

    bool is_icache_enabled() noexcept
    {
        uint32_t sctlr;
        __asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
        return (sctlr & (1 << 12)) != 0;
    }

    bool is_dcache_enabled() noexcept
    {
        uint32_t sctlr;
        __asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
        return (sctlr & (1 << 2)) != 0;
    }
}

