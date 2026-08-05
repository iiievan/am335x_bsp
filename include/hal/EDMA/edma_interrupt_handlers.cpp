#include "InterruptDispatcher.hpp"
#include "hal/EDMA/EDMA.hpp"

extern "C" void EDMA_Completion_ISR()
{
    // 1. Считываем флага завершения (IPR)
    uint32_t ipr = HAL::EDMA::get_intr_status();
    uint32_t iprh = HAL::EDMA::intr_status_high_get();

    // 2. Обрабатываем каналы 0..31
    while (ipr != 0)
    {
        const uint32_t tcc = __builtin_ctz(ipr); // Находим номер первого установленного бита (Count Trailing Zeros)
        
        // Очищаем прерывание в железе
        HAL::EDMA::clr_intr(tcc);

        // Диспетчеризируем в соответствующий объект C++
        HAL::EDMA::InterruptDispatcher::dispatchCompletion(static_cast<uint8_t>(tcc));

        ipr &= ~(1u << tcc); // Сбрасываем обработанный бит
    }

    // 3. Обрабатываем каналы 32..63 (если используются)
    while (iprh != 0)
    {
        const uint32_t tcc_high = __builtin_ctz(iprh);
        const uint32_t tcc = tcc_high + 32;

        HAL::EDMA::clr_intr(tcc);
        HAL::EDMA::InterruptDispatcher::dispatchCompletion(static_cast<uint8_t>(tcc));

        iprh &= ~(1u << tcc_high);
    }
}

extern "C" void EDMA_Error_ISR()
{
    // 1. Проверяем пропущенные события (Event Missed Register)
    uint32_t emr = HAL::EDMA::get_Err_intr_status();
    if (emr != 0)
    {
        while (emr != 0)
        {
            const uint32_t ch = __builtin_ctz(emr);
            
            // Очищаем ошибку
            HAL::EDMA::clr_miss_evt(ch);
            
            // Оповещаем подписчика
            HAL::EDMA::InterruptDispatcher::dispatchError(ch, HAL::EDMA::ErrorType::EventMissed);
            
            emr &= ~(1u << ch);
        }
    }

    // 2. Проверяем ошибки контроллера (CCERR - Threshold, TCC error)
    const uint32_t ccerr = HAL::EDMA::get_CC_Err_status();
    if (ccerr != 0)
    {
        HAL::EDMA::clr_CC_Err(ccerr);
        HAL::EDMA::CC_Err_evaluate(); // Re-evaluate logic
    }
}