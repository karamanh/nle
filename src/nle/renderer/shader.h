/**
 * @file shader.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <GL/glew.h>

#include <string>
#include <future>

namespace nle
{

enum class shader_source
{
    file,
    memory
};

enum class shader_type
{
    vertex = GL_VERTEX_SHADER,
    fragment = GL_FRAGMENT_SHADER,
    geometry = GL_GEOMETRY_SHADER
};

class shader
{
public:
    shader() = delete;
    shader(const std::string& vertex_source, const std::string& fragment_source, enum shader_source source = shader_source::file);
    
    void load();

    void use();

    void unload();
    
    unsigned int program() const;

    unsigned int uniform_location(const std::string& uniform) const;

private:
    GLuint m_program;

    GLuint m_vertex_shader;

    GLuint m_fragment_shader;

    GLuint m_geometry_shader;

    /// vertex shader source
    std::string m_vertex_source;

    /// fragment shader source
    std::string m_fragment_source;

    /// geometry shader source
    std::string m_geometry_source;

    std::future<bool> m_ready;

    void compile_shaders();

    void attach_shaders(enum shader_type type);

    friend class renderer_3d;
};
} // namespace nle
