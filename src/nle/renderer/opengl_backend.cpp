/**
 * @file opengl_backend.cpp
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief OpenGL backend implementation
 * @version 0.1
 * @date 2024-11-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "opengl_backend.h"
#include "shader.h"
#include "texture.h"
#include <glm/gtc/type_ptr.hpp>

namespace nle
{

opengl_backend::opengl_backend()
{
}

opengl_backend::~opengl_backend()
{
}

void opengl_backend::execute_commands(const render_command_buffer& command_buffer)
{
    for (const auto& cmd : command_buffer.commands())
    {
        execute_command(cmd);
    }
}

void opengl_backend::execute_command(const render_command& cmd)
{
    switch (cmd.type)
    {
        case command_type::set_polygon_mode:
        {
            auto data = std::get<polygon_mode_data>(cmd.data);
            glPolygonMode(data.front_and_back, data.mode);
            break;
        }
        case command_type::use_shader:
        {
            auto shader = std::get<ref<class shader>>(cmd.data);
            if (shader)
            {
                shader->use();
            }
            break;
        }
        case command_type::set_uniform_matrix4:
        {
            auto uniform = std::get<uniform_data>(cmd.data);
            // Note: We need the current shader context for this to work
            // In a real implementation, you might want to track the current shader
            auto matrix = std::get<glm::mat4>(uniform.value);
            GLint program;
            glGetIntegerv(GL_CURRENT_PROGRAM, &program);
            GLint location = glGetUniformLocation(program, uniform.name.c_str());
            if (location != -1)
            {
                glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
            }
            break;
        }
        case command_type::set_uniform_vec3:
        {
            auto uniform = std::get<uniform_data>(cmd.data);
            auto vec = std::get<glm::vec3>(uniform.value);
            GLint program;
            glGetIntegerv(GL_CURRENT_PROGRAM, &program);
            GLint location = glGetUniformLocation(program, uniform.name.c_str());
            if (location != -1)
            {
                glUniform3f(location, vec.x, vec.y, vec.z);
            }
            break;
        }
        case command_type::set_uniform_float:
        {
            auto uniform = std::get<uniform_data>(cmd.data);
            auto value = std::get<float>(uniform.value);
            GLint program;
            glGetIntegerv(GL_CURRENT_PROGRAM, &program);
            GLint location = glGetUniformLocation(program, uniform.name.c_str());
            if (location != -1)
            {
                glUniform1f(location, value);
            }
            break;
        }
        case command_type::set_uniform_int:
        {
            auto uniform = std::get<uniform_data>(cmd.data);
            auto value = std::get<int>(uniform.value);
            GLint program;
            glGetIntegerv(GL_CURRENT_PROGRAM, &program);
            GLint location = glGetUniformLocation(program, uniform.name.c_str());
            if (location != -1)
            {
                glUniform1i(location, value);
            }
            break;
        }
        case command_type::use_texture:
        {
            auto texture = std::get<ref<class texture>>(cmd.data);
            if (texture)
            {
                texture->use();
            }
            break;
        }
        case command_type::draw_elements:
        {
            auto data = std::get<draw_elements_data>(cmd.data);
            glBindVertexArray(data.vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
            glDrawElements(data.primitive_type, data.count, GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            break;
        }
        case command_type::set_depth_mask:
        {
            auto mask = std::get<bool>(cmd.data);
            glDepthMask(mask ? GL_TRUE : GL_FALSE);
            break;
        }
    }
}

} // namespace nle
