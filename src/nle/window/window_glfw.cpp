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

#include <stdexcept>

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
    while (!glfwWindowShouldClose(m_handle))
    {
        glfwGetWindowSize(m_handle, &m_width, &m_height);

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
    }
    m_closed = true;
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

} // namespace nle
