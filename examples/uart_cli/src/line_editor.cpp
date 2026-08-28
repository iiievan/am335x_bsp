#include "line_editor.hpp"

[[nodiscard]] bool strings_equal(const char* lhs, const char* rhs) noexcept
{
    while (*lhs != '\0' && *rhs != '\0')
    {
        if (*lhs++ != *rhs++)
            return false;
    }
    return *lhs == *rhs;
}

[[nodiscard]] std::size_t string_length(const char* string) noexcept
{
    std::size_t length = 0u;
    while (string[length] != '\0')
        ++length;
    return length;
}

void copy_string(char* destination, const char* source) noexcept
{
    std::size_t i = 0u;
    while (i + 1u < LineEditor::COMMAND_BUFFER_SIZE && source[i] != '\0')
    {
        destination[i] = source[i];
        ++i;
    }
    destination[i] = '\0';
}

LineEditor::Result LineEditor::read_line(char* command) noexcept
{
    std::size_t length = 0u;
    std::size_t cursor = 0u;
    std::size_t history_view = m_history_count;
    char draft[COMMAND_BUFFER_SIZE]{};

    command[0] = '\0';
    m_uart.put_string(CLI_PROMPT);

    while (true)
    {
        const char c = m_uart.get_char();

        if (m_ignore_lf && c == '\n')
        {
            m_ignore_lf = false;
            continue;
        }
        m_ignore_lf = false;

        if (c == '\r' || c == '\n')
        {
            m_ignore_lf = c == '\r';
            m_uart.put_string("\n");
            add_to_history(command);
            return Result::SUBMITTED;
        }

        if (static_cast<uint8_t>(c) == 0x03u) // Ctrl+C
        {
            m_uart.put_string("^C\n");
            command[0] = '\0';
            return Result::CANCELLED;
        }

        if (static_cast<uint8_t>(c) == 0x0Cu) // Ctrl+L
        {
            m_uart.put_string("\x1b[2J\x1b[H");
            redraw(command, length, cursor);
            continue;
        }

        if (static_cast<uint8_t>(c) == 0x1Bu) // ESC / ANSI sequence
        {
            handle_escape(command, length, cursor, history_view, draft);
            continue;
        }

        if (c == '\b' || static_cast<uint8_t>(c) == 0x7Fu)
        {
            if (cursor != 0u)
            {
                for (std::size_t i = cursor - 1u; i < length; ++i)
                    command[i] = command[i + 1u];
                --cursor;
                --length;
                history_view = m_history_count;
                redraw(command, length, cursor);
            }
            continue;
        }

        const uint8_t byte = static_cast<uint8_t>(c);
        if (byte < 0x20u || byte > 0x7Eu)
            continue;

        if (length + 1u >= COMMAND_BUFFER_SIZE)
        {
            m_uart.put_char('\a');
            continue;
        }

        for (std::size_t i = length; i > cursor; --i)
            command[i] = command[i - 1u];
        command[cursor++] = c;
        command[++length] = '\0';
        history_view = m_history_count;
        redraw(command, length, cursor);
    }
}

void LineEditor::redraw(const char* command, const std::size_t length, const std::size_t cursor) noexcept
{
    m_uart.put_char('\r');
    m_uart.put_string("\x1b[2K");
    m_uart.put_string(CLI_PROMPT);
    m_uart.put_data(command, length);

    for (std::size_t i = cursor; i < length; ++i)
        m_uart.put_char('\b');
}

void LineEditor::load_line(char* command,
                           std::size_t& length,
                           std::size_t& cursor,
                           const char* source) noexcept
{
    copy_string(command, source);
    length = string_length(command);
    cursor = length;
    redraw(command, length, cursor);
}

void LineEditor::add_to_history(const char* command) noexcept
{
    if (*command == '\0')
        return;
    if (m_history_count != 0u &&
        strings_equal(m_history[m_history_count - 1u], command))
    {
        return;
    }

    if (m_history_count < HISTORY_DEPTH)
    {
        copy_string(m_history[m_history_count], command);
        ++m_history_count;
        return;
    }

    for (std::size_t i = 1u; i < HISTORY_DEPTH; ++i)
        copy_string(m_history[i - 1u], m_history[i]);
    copy_string(m_history[HISTORY_DEPTH - 1u], command);
}

void LineEditor::history_up(char* command,
                            std::size_t& length,
                            std::size_t& cursor,
                            std::size_t& history_view,
                            char* draft) noexcept
{
    if (m_history_count == 0u || history_view == 0u)
    {
        m_uart.put_char('\a');
        return;
    }

    if (history_view == m_history_count)
        copy_string(draft, command);
    --history_view;
    load_line(command, length, cursor, m_history[history_view]);
}

void LineEditor::history_down(char* command,
                              std::size_t& length,
                              std::size_t& cursor,
                              std::size_t& history_view,
                              const char* draft) noexcept
{
    if (history_view >= m_history_count)
    {
        m_uart.put_char('\a');
        return;
    }

    ++history_view;
    load_line(command,
              length,
              cursor,
              history_view == m_history_count ? draft : m_history[history_view]);
}

void LineEditor::erase_at_cursor(char* command, std::size_t& length, const std::size_t cursor) noexcept
{
    if (cursor >= length)
        return;
    for (std::size_t i = cursor; i < length; ++i)
        command[i] = command[i + 1u];
    --length;
    redraw(command, length, cursor);
}

void LineEditor::handle_escape(char* command,
                               std::size_t& length,
                               std::size_t& cursor,
                               std::size_t& history_view,
                               char* draft) noexcept
{
    const char prefix = m_uart.get_char();
    if (prefix != '[' && prefix != 'O')
        return;

    const char code = m_uart.get_char();
    switch (code)
    {
    case 'A': // Up
        history_up(command, length, cursor, history_view, draft);
        break;
    case 'B': // Down
        history_down(command, length, cursor, history_view, draft);
        break;
    case 'C': // Right
        if (cursor < length)
        {
            m_uart.put_char(command[cursor]);
            ++cursor;
        }
        else
        {
            m_uart.put_char('\a');
        }
        break;
    case 'D': // Left
        if (cursor != 0u)
        {
            m_uart.put_char('\b');
            --cursor;
        }
        else
        {
            m_uart.put_char('\a');
        }
        break;
    case 'H': // Home
        while (cursor != 0u)
        {
            m_uart.put_char('\b');
            --cursor;
        }
        break;
    case 'F': // End
        while (cursor < length)
            m_uart.put_char(command[cursor++]);
        break;
    case '1': // Home: ESC [ 1 ~
    case '3': // Delete: ESC [ 3 ~
    case '4': // End: ESC [ 4 ~
    case '7': // Home: ESC [ 7 ~
    case '8': // End: ESC [ 8 ~
        if (m_uart.get_char() != '~')
            return;
        if (code == '3')
            erase_at_cursor(command, length, cursor);
        else if (code == '1' || code == '7')
        {
            while (cursor != 0u)
            {
                m_uart.put_char('\b');
                --cursor;
            }
        }
        else
        {
            while (cursor < length)
                m_uart.put_char(command[cursor++]);
        }
        break;
    default:
        break;
    }
}
