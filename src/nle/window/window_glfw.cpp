/**
 * @file window_glfw.cpp
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "window_glfw.h"
#include "../core/utils.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <thread>

namespace nle
{
window_glfw::window_glfw(unsigned int width, unsigned int height, const std::string& title)
{
    GLenum err = glfwInit();
    if (err != GLFW_TRUE)
    {
        throw std::runtime_error("fatal_error: could not initialize glfw. error code: " + std::to_string(err));
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(m_handle);

    err = glewInit();
    if (err != GLEW_OK)
    {
        throw std::runtime_error("fatal_error: could not initialize glew. error code: " + std::to_string(err));
    }

    m_input_handler = make_ref<input_handler_glfw>(m_handle);

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    utils::print((char*)vendor);
    utils::print((char*)renderer);

    glfwSetWindowUserPointer(m_handle, this);
    
    // mouse & keyboard handlers
}

window_glfw::~window_glfw()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void window_glfw::display()
{
    double last_frame = glfwGetTime();

    while (!glfwWindowShouldClose(m_handle))
    {
        const double now = glfwGetTime();
        const float delta_time = static_cast<float>(now - last_frame);
        last_frame = now;

        glfwGetWindowSize(m_handle, &m_width, &m_height);

        if(update())
        {
            update()(delta_time);
        }

        if(render_3d())
        {
            render_3d()();
        }

        if(render_ui())
        {
            render_ui()();
        }

        m_input_handler->poll_keyboard_input();
        m_input_handler->poll_mouse_input();
        
        glfwSwapBuffers(m_handle);
        glfwPollEvents();

        pace(now);
    }
    m_closed = true;
}

void window_glfw::set_frame_limit(int fps, int unfocused)
{
    m_frame_limit = fps > 0 ? fps : 0;
    m_unfocused_limit = unfocused > 0 ? unfocused : 0;
}

void window_glfw::set_vsync(bool on)
{
    glfwMakeContextCurrent(m_handle);
    glfwSwapInterval(on ? 1 : 0);
}

void window_glfw::pace(double frame_started)
{
    // Minimised, nobody sees anything; behind another window, barely.
    int limit = m_frame_limit;

    if(glfwGetWindowAttrib(m_handle, GLFW_ICONIFIED))
    {
        limit = 5;
    }
    else if(!glfwGetWindowAttrib(m_handle, GLFW_FOCUSED) && m_unfocused_limit > 0)
    {
        limit = limit > 0 ? std::min(limit, m_unfocused_limit) : m_unfocused_limit;
    }

    if(limit <= 0)
    {
        return;
    }

    const double frame = 1.0 / static_cast<double>(limit);

    // Slept for most of what is left, and the last millisecond waited out by
    // yielding, since a sleep may overshoot by about that much.
    for(;;)
    {
        const double left = frame - (glfwGetTime() - frame_started);

        if(left <= 0.0)
        {
            break;
        }

        if(left > 0.002)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(left - 0.0015));
        }
        else
        {
            std::this_thread::yield();
        }
    }
}

void window_glfw::close()
{
    glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
}

void window_glfw::set_fullscreen(bool fullscreen)
{
    if((m_fullscreen = fullscreen))
    {
        const GLFWvidmode * vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(m_handle, glfwGetPrimaryMonitor(), 0, 0, vm->width, vm->height, vm->refreshRate);
    }
    else
    {
        glfwSetWindowMonitor(m_handle, nullptr, 0, 0, NLE_WINDOW_DEFAULT_WIDTH, NLE_WINDOW_DEFAULT_HEIGHT, 0);
    }
}

bool window_glfw::fullscreen()
{
    return m_fullscreen;
}

void window_glfw::set_cursor_visibility(bool visible)
{
    glfwSetInputMode(m_handle, GLFW_CURSOR, (m_cursor_visible = visible) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

bool window_glfw::cursor_visibility()
{
    return m_cursor_visible;
}

ref<input_handler_glfw> window_glfw::input_handler()
{
    return m_input_handler;
}

GLFWwindow* window_glfw::handle() const
{
    return m_handle;
}

} // namespace nle
