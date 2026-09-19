/**
 * @file ui.hpp
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../core/ref.h"
#include "../window/window.hpp"

namespace nle
{

template <typename winhandle_type>
class ui
{
protected:
    winhandle_type m_render_target;

    virtual void render_helper()
    {
        render();
    }


public:
    ui(winhandle_type render_target)
        : m_render_target(render_target)
    {}

    virtual void render() = 0;
};

} // namespace nle
