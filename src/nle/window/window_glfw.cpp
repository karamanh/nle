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

        render_routine()(); // or m_render_routine() if this is too annoying :P
        
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

} // namespace nle
