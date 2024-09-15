/**
 * @file clock.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief
 * @version 0.1
 * @date 2024-08-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <chrono>

namespace nle
{

    class clock
    {
    public:
        clock();

        int64_t elapsed_time_s();
        int64_t elapsed_time_ms();
        int64_t elapsed_time_us();

        void restart();

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_tp_start;
    };

} // namespace osd
