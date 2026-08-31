#ifndef UART_SHELL_CONSOLE_HPP
#define UART_SHELL_CONSOLE_HPP

#include <cstddef>

namespace SHELL
{
    class Console final
    {
    public:
        template<class Uart>
        explicit Console(Uart& uart) noexcept
            : m_context(&uart),
              m_get_char([](void* context) noexcept {
                  return static_cast<Uart*>(context)->get_char();
              }),
              m_put_char([](void* context, const char value) noexcept {
                  static_cast<Uart*>(context)->put_char(value);
              }),
              m_put_string([](void* context, const char* value) noexcept {
                  static_cast<Uart*>(context)->put_string(value);
              }),
              m_put_data([](void* context, const void* data,
                            const std::size_t size) noexcept {
                  static_cast<Uart*>(context)->put_data(data, size);
              }) {}

        [[nodiscard]] char get_char() const noexcept { return m_get_char(m_context); }
        void put_char(const char value) const noexcept { m_put_char(m_context, value); }
        void put_string(const char* value) const noexcept { m_put_string(m_context, value); }
        void put_data(const void* data, const std::size_t size) const noexcept
        {
            m_put_data(m_context, data, size);
        }

    private:
        void* m_context;
        char (*m_get_char)(void*) noexcept;
        void (*m_put_char)(void*, char) noexcept;
        void (*m_put_string)(void*, const char*) noexcept;
        void (*m_put_data)(void*, const void*, std::size_t) noexcept;
    };
}

#endif
