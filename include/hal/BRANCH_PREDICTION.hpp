/*=======================================================================*/
/*  hal_mmu_cache.hpp - MMU & Cache initialization for AM335x           */
/*=======================================================================*/
#ifndef __BRANCH_PREDICTION_HPP
#define __BRANCH_PREDICTION_HPP

namespace HAL::BRANCH_PREDICTION
{
    void enable() noexcept;
    void disable() noexcept;
    void invalidate() noexcept;
    bool is_enabled() noexcept;
}

#endif // __BRANCH_PREDICTION_HPP