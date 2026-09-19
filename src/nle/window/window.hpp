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

    virtual void set_cursor_visibility(bool visible) = 0;
    virtual bool cursor_visibility() = 0;

    int width() { return m_width; }
    int height() { return m_height; }
    bool closed() { return m_closed; }
    std::function<void()>& render_3d() { return m_render_3d; }
    std::function<void()>& render_ui() { return m_render_ui; }

    /**
     * @brief Game logic hook, called once per frame before rendering.
     *
     * The argument is the time in seconds since the previous frame. This is
     * where per-frame work belongs; doing it on another thread, as the old
     * demo did, races with the renderer reading the same objects.
     */
    std::function<void(float)>& update() { return m_update; }

protected:
    handle_type m_handle;
    int m_width;
    int m_height;
    bool m_closed = false;
    bool m_fullscreen = false;
    bool m_cursor_visible = false;
    std::function<void()> m_render_3d;
    std::function<void()> m_render_ui;
    std::function<void(float)> m_update;
};

} // namespace nle
