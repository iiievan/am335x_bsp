#include "InterruptDispatcher.hpp"
#include "hal/EDMA/EDMA.hpp"

extern "C" void EDMA_Completion_ISR()
{
    // Read all completion flags (IPR and IPRH)
    uint32_t ipr = HAL::EDMA::get_intr_status();
    uint32_t iprh = HAL::EDMA::intr_status_high_get();

    // Process channels 0..31
    while (ipr != 0)
    {
        const uint32_t tcc = __builtin_ctz(ipr); // Find the number of the first set bit (Count Trailing Zeros)
        HAL::EDMA::clr_intr(tcc);
        HAL::EDMA::InterruptDispatcher::dispatchCompletion(static_cast<uint8_t>(tcc));
        ipr &= ~(1u << tcc); // Clear processed bit
    }

    // Process channels 32..63 (if used)
    while (iprh != 0)
    {
        const uint32_t tcc_high = __builtin_ctz(iprh);
        const uint32_t tcc = tcc_high + 32u;
        HAL::EDMA::clr_intr(tcc);
        HAL::EDMA::InterruptDispatcher::dispatchCompletion(static_cast<uint8_t>(tcc));
        iprh &= ~(1u << tcc_high);
    }

    HAL::EDMA::revaluateInterruptLine();
}

extern "C" void EDMA_Error_ISR()
{
    // Check EMR and EMRH (DMA Event Missed Register)
    uint32_t emr = HAL::EDMA::get_Err_intr_status();
    uint32_t emrh = HAL::EDMA::get_Err_intr_high_status();

    // dispatch errors on channels 0..31
    while (emr != 0)
    {
        const uint32_t ch = __builtin_ctz(emr);
        HAL::EDMA::clr_miss_evt(ch);
        HAL::EDMA::InterruptDispatcher::dispatchError(ch, HAL::EDMA::ErrorType::EventMissed);
        emr &= ~(1u << ch);
    }

    // dispatch errors on channels 32..63
    while (emrh != 0)
    {
        const uint32_t bit = __builtin_ctz(emrh);
        const uint32_t ch  = 32u + bit;

        HAL::EDMA::clr_miss_evt(ch);
        HAL::EDMA::InterruptDispatcher::dispatchError(static_cast<uint8_t>(ch),HAL::EDMA::ErrorType::EventMissed);

        emrh &= ~(1u << bit);
    }

    uint32_t qemr = HAL::EDMA::QDMA_get_Err_intr_status();
    while (qemr != 0u)
    {
        const uint32_t qch = __builtin_ctz(qemr);
        const uint32_t param_id = REGS::EDMA::AM335X_EDMA3CC->QCHMAP[qch].b.PAENTRY;
        HAL::EDMA::QDMA_clr_miss_evt(qch);

        if (param_id < REGS::EDMA::AM335x_PARAMSETS_MAX)
        {
            const uint32_t tcc = REGS::EDMA::AM335X_EDMA3CC->paRAM(param_id).OPT.b.TCC;
            HAL::EDMA::InterruptDispatcher::dispatchError(static_cast<uint8_t>(tcc), HAL::EDMA::ErrorType::EventMissed);
        }
        qemr &= ~(1u << qch);
    }

    const uint32_t ccerr = HAL::EDMA::get_CC_Err_status();
    if (ccerr != 0)
    {
        HAL::EDMA::clr_CC_Err(ccerr);
    }

    HAL::EDMA::CC_Err_evaluate(); // Re-evaluate logic
}