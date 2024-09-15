#include "nle/core/clock.h"

nle::clock::clock()
    : m_tp_start(std::chrono::high_resolution_clock::now())
{
}

int64_t nle::clock::elapsed_time_s()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - m_tp_start).count();
}

int64_t nle::clock::elapsed_time_ms()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_tp_start).count();
}

int64_t nle::clock::elapsed_time_us()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - m_tp_start).count();
}

void nle::clock::restart()
{
    m_tp_start = std::chrono::high_resolution_clock::now();
}
