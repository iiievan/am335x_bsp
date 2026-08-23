/*=======================================================================*/
/*  hal_mmu_cache.hpp - MMU & Cache initialization for AM335x           */
/*=======================================================================*/
#ifndef __CACHE_HPP
#define __CACHE_HPP

#include <cstdint>

namespace HAL::CACHE
{
    enum class Type : uint32_t
    {
        ICACHE = 0x01,
        DCACHE = 0x02,
        ALL    = 0x03
    };

    void init() noexcept;
    void enable(Type cache_type = Type::ALL) noexcept;
    void disable(Type cache_type = Type::ALL) noexcept;

    // I-Cache
    void icache_invalidate_all() noexcept;
    void icache_invalidate_range(uint32_t start_addr, uint32_t size) noexcept;

    // D-Cache (All)
    void dcache_clean_all() noexcept;
    void dcache_invalidate_all() noexcept;
    void dcache_clean_invalidate_all() noexcept;

    // D-Cache (Range)
    void dcache_clean_range(uint32_t start_addr, uint32_t size) noexcept;
    void dcache_invalidate_range(uint32_t start_addr, uint32_t size) noexcept;
    void dcache_clean_invalidate_range(uint32_t start_addr, uint32_t size) noexcept;

    bool is_icache_enabled() noexcept;
    bool is_dcache_enabled() noexcept;
}

#endif // __CACHE_HPP