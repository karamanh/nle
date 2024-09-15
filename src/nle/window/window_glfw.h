/**
 * @file window_glfw.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <functional>

#include "window.hpp"
#include "input_handler_glfw.h"
#include "../core/ref.h"
#include "../ui/ui.hpp"

namespace nle
{

class window_glfw : public window<GLFWwindow*> {
public:
    window_glfw(unsigned int width, unsigned int height, const std::string& title = NLE_WINDOW_DEFAULT_TITLE);
    ~window_glfw();

    void display();
    void close();

    void set_fullscreen(bool fullscreen);
    bool fullscreen();

    void set_cursor_visibility(bool visible);
    bool cursor_visibility();

    ref<input_handler_glfw> input_handler();

private:
    ref<input_handler_glfw> m_input_handler;
};

} // namespace nle
