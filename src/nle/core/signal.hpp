#pragma once

#include <thread>
#include <functional>

namespace nle
{

template <typename T, typename ...ExtraArgs>
class signal
{
public:
    typedef std::function<void(const T & message, ExtraArgs... args)> callback_t;

    void emit(const T &message, ExtraArgs... args, bool async = true)
    {
        for (const auto &it : m_callbacks)
        {
            if(async)
            {
                std::thread(it, message, args...).detach();
            }
            else
            {
                it(message, args...);
            }
        }
    }

    void bind_callback(callback_t callback)
    {
        m_callbacks.push_back(callback);
    }

    void unbind_all()
    {
        m_callbacks.clear();
    }

private:
    std::vector<callback_t> m_callbacks;
};

} // namespace nles
