/**
 * @file opengl_backend.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief OpenGL backend for executing render commands
 * @version 0.1
 * @date 2024-11-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "render_command.h"
#include <GL/glew.h>

namespace nle
{

class opengl_backend
{
public:
    opengl_backend();
    ~opengl_backend();

    void execute_commands(const render_command_buffer& command_buffer);

private:
    void execute_command(const render_command& cmd);
};

} // namespace nle
