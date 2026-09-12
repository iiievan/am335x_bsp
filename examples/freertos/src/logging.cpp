#include "logging.hpp"
#include "log/log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
extern "C" volatile uint32_t ulPortInterruptNesting;
#if AM335X_FREERTOS_LOG_RTT
#include "log/sinks/RttSink.hpp"
#endif
#if AM335X_FREERTOS_LOG_UART
#include "hal/boards/beaglebone_black.hpp"
#include "log/sinks/UartSink.hpp"
#endif

namespace
{
    class Session final
    {
        SemaphoreHandle_t m_mutex{nullptr};
#if AM335X_FREERTOS_LOG_RTT
        HAL::LOG::RttSink m_rtt;
#endif
#if AM335X_FREERTOS_LOG_UART
        HAL::LOG::UartSink<HAL::UART::uart0_t> m_uart{Board::get_uart0()};
#endif
        static int acquire(void* context) noexcept
        {
            // This AM335x port runs IRQ dispatch in SVC mode, so CPSR alone
            // cannot distinguish a task from an interrupt handler.
            if (ulPortInterruptNesting != 0u)
                return 0;
#if defined(__arm__)
            uint32_t cpsr;
            __asm volatile("mrs %0, cpsr" : "=r"(cpsr));
            const auto mode = cpsr & 0x1fu;
            if (mode != 0x10u && mode != 0x13u && mode != 0x1fu)
                return 0; // IRQ/FIQ/Abort/Undefined: no formatter or RTOS calls.
#endif
            auto& self = *static_cast<Session*>(context);
            const auto state = xTaskGetSchedulerState();
            if (state == taskSCHEDULER_NOT_STARTED)
                return 1;
            if (state != taskSCHEDULER_RUNNING || self.m_mutex == nullptr)
                return 0;
#if defined(__arm__)
            if ((cpsr & 0x80u) != 0u)
                return 0; // Never block while interrupts are masked.
#endif
            return xSemaphoreTake(self.m_mutex, portMAX_DELAY) == pdTRUE ? 2 : 0;
        }
        static void release(void* context, int token) noexcept
        {
            if (token == 2)
                (void)xSemaphoreGive(static_cast<Session*>(context)->m_mutex);
        }
    public:
        Session() noexcept
        {
            (void)log_set_lock(this, acquire, release);
        }
        ~Session() noexcept
        {
            // Only valid after all logging/tasks are quiescent (normal firmware
            // never destroys this static session while its scheduler runs).
            (void)log_set_lock(nullptr, nullptr, nullptr);
            if (m_mutex != nullptr)
                vSemaphoreDelete(m_mutex);
        }
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;
        bool prepare_scheduler() noexcept
        {
            if (m_mutex == nullptr)
                m_mutex = xSemaphoreCreateMutex();
            return m_mutex != nullptr;
        }
        bool ready() const noexcept
        {
            bool result = true;
#if AM335X_FREERTOS_LOG_RTT
            result = result && m_rtt.registered();
#endif
#if AM335X_FREERTOS_LOG_UART
            result = result && m_uart.registered();
#endif
            return result;
        }
    };
    Session& session() noexcept
    {
        static Session instance;
        return instance;
    }
}

bool App::Logging::init() noexcept
{
    return session().ready();
}

bool App::Logging::prepare_scheduler() noexcept
{
    return session().prepare_scheduler();
}
