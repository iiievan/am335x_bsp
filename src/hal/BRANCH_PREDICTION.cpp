/*=======================================================================*/
/*  hal_mmu_cache.cpp - MMU & Cache implementation                      */
/*=======================================================================*/
#include "hal/BRANCH_PREDICTION.hpp"
#include "startup/cp15.h"
#include "stdint.h"

namespace HAL::BRANCH_PREDICTION
{

    void enable() noexcept
    {
        cp15_branch_prediction_enable();
        cp15_DSB_ISB_sync_barrier();
    }

    void disable() noexcept
    {
        cp15_branch_prediction_disable();
        cp15_DSB_ISB_sync_barrier();
    }

    void invalidate() noexcept
    {
        cp15_branch_predictor_invalidate();
    }

    bool is_enabled() noexcept
    {
        uint32_t sctlr;
        __asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
        return (sctlr & (1 << 11)) != 0;
    }
}

