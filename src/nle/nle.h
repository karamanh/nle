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

#include <string>

namespace nle
{

class nle
{
public:
    /**
     * @brief Opens a window and a renderer for it.
     *
     * The default is small on purpose -- it is what a demo wants. A tool with
     * eight panels down its sides wants rather more, and asking for it here
     * rather than resizing afterwards matters: anything that lays itself out
     * relative to the window does so the first time it is drawn, and would
     * lay itself out for the small one.
     */
    nle(unsigned int width = NLE_WINDOW_DEFAULT_WIDTH,
        unsigned int height = NLE_WINDOW_DEFAULT_HEIGHT,
        const std::string& title = NLE_WINDOW_DEFAULT_TITLE);

    ~nle();

    ref<class window_glfw> window();

    ref<class renderer_3d> renderer_3d(); 

    void run();

private:
    ref<class window_glfw> m_window;
    ref<class renderer_3d> m_renderer;
};

} // namespace nle
