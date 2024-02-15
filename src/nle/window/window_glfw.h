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

#include "window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <functional>

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

private:
};

} // namespace nle
