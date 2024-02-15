/**
 * @file window.hpp
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <functional>

namespace nle
{

#define NLE_WINDOW_DEFAULT_WIDTH    480
#define NLE_WINDOW_DEFAULT_HEIGHT   480
#define NLE_WINDOW_DEFAULT_TITLE    "nice little engine"

template <typename handle_type>
class window
{
public:
    window() = default;
    virtual ~window(){}

    virtual void display() = 0;
    virtual void close() = 0;

    virtual void set_fullscreen(bool fullscreen) = 0;
    virtual bool fullscreen() = 0;

    int width() { return m_width; }
    int height() { return m_height; }
    bool closed() { return m_closed; }
    std::function<void()>& render_routine() { return m_render_routine; }

protected:
    handle_type m_handle;
    int m_width;
    int m_height;
    bool m_closed;
    bool m_fullscreen;
    std::function<void()> m_render_routine;
};

} // namespace nle
