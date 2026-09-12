#ifndef FREERTOS_EXAMPLE_LOGGING_HPP
#define FREERTOS_EXAMPLE_LOGGING_HPP

namespace App::Logging
{
    // Once, before scheduler start. Owns registrations for firmware lifetime.
    [[nodiscard]] bool init() noexcept;
    // After board/INTC initialization, before creating/starting tasks.
    [[nodiscard]] bool prepare_scheduler() noexcept;
}

#endif
