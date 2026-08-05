#ifndef HAL_INTERRUPTDISPATCHER_HPP
#define HAL_INTERRUPTDISPATCHER_HPP

#include <cstdint>
#include <functional> // Можно использовать inplace_function или обычные функции-указатели

#include "regs/EDMA.hpp"

namespace HAL::EDMA
{
    using Callback_t = void(*)(void* context);

    enum class ErrorType : uint8_t
    {
        EventMissed,       // Пропущено событие (EMR/QEMR)
        ThresholdExceeded, // Переполнение очереди FIFO (CCERR)
        BusError,          // Ошибка шины при чтении/записи (TC ERR)
        TransferRequest,    // TR detected that violates constant addressing mode transfer
        MemoryError,        //  User attempted to read or write to an invalid address in configuration memory map
    };

    using ErrorCallback_t = void(*)(ErrorType err, void* context);

    class InterruptDispatcher
    {
    private:
        struct ChannelHandler
        {
            Callback_t      onComplete;
            ErrorCallback_t onError;
            void*           context;

            ChannelHandler() = default;
        };

        static inline ChannelHandler handlers[REGS::EDMA::AM335X_DMACH_MAX];

    public:
        static void registerHandler(const uint8_t tcc,
                                    const Callback_t onComplete,
                                    const ErrorCallback_t onError,
                                    void* context = nullptr) noexcept
        {
            if (tcc < 64)
            {
                handlers[tcc].onComplete = onComplete;
                handlers[tcc].onError    = onError;
                handlers[tcc].context    = context;
            }
        }

        static void unregisterHandler(const uint8_t tcc) noexcept
        {
            if (tcc < 64)
            {
                handlers[tcc] = {};
            }
        }

        // Call from C-style ISR
        static void dispatchCompletion(const uint8_t tcc) noexcept
        {
            if (tcc < 64 && handlers[tcc].onComplete)
            {
                handlers[tcc].onComplete(handlers[tcc].context);
            }
        }

        // Call from C-style ISR
        static void dispatchError(const uint8_t ch_or_tcc, const ErrorType err) noexcept
        {
            if (ch_or_tcc < 64 && handlers[ch_or_tcc].onError)
            {
                handlers[ch_or_tcc].onError(err, handlers[ch_or_tcc].context);
            }
        }
    };
}
#ifdef __cplusplus
extern "C" {
#endif

void EDMA_Completion_ISR();
void EDMA_Error_ISR();

#ifdef __cplusplus
}
#endif

#endif //HAL_INTERRUPTDISPATCHER_HPP