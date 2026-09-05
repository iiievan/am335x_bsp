#ifndef AM335X_LOG_RTT_SINK_HPP
#define AM335X_LOG_RTT_SINK_HPP

namespace HAL::LOG
{
    bool rtt_backend_init() noexcept;
    void rtt_backend_deinit() noexcept;

    class RttSink final
    {
        bool m_registered;

    public:
        RttSink() noexcept : m_registered(rtt_backend_init()) { }
        ~RttSink() noexcept
        {
            if (m_registered)
                rtt_backend_deinit();
        }

        RttSink(const RttSink&) = delete;
        RttSink& operator=(const RttSink&) = delete;
        RttSink(RttSink&&) = delete;
        RttSink& operator=(RttSink&&) = delete;

        [[nodiscard]] bool registered() const noexcept
        {
            return m_registered;
        }
    };
}

#endif
