/**
 * @file nle.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "core/ref.h"
#include "window/window_glfw.h"
#include "renderer/renderer_3d.h"

namespace nle
{

class nle
{
public:
    nle();
    ~nle();

    ref<class window_glfw> window_glfw();

    ref<class renderer_3d> renderer_3d(); 

    void run();

private:
    ref<class window_glfw> m_window;
    ref<class renderer_3d> m_renderer;
};

} // namespace nle
