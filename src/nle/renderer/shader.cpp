#include "shader.h"
#include "../core/utils.h"

namespace nle
{
    shader::shader(const std::string &vertex_source, const std::string &fragment_source, enum shader_source source)
    {
        if(source == shader_source::file)
        {
            m_vertex_source = utils::load_file(vertex_source);
            m_fragment_source = utils::load_file(fragment_source);
        }
        else if(source == shader_source::memory)
        {
            // we treat params vertex_shader and fragment_shader as sources directly
            m_vertex_source = vertex_source;
            m_fragment_source = fragment_source;
        }
    }

    void shader::load()
    {
        compile_shaders();
    }

    void shader::use()
    {
        glUseProgram(m_program);
    }

    void shader::unload()
    {
        if(m_vertex_shader != 0)
        {
            glDeleteShader(m_vertex_shader);
        }
        else if(m_fragment_shader != 0)
        {
            glDeleteShader(m_fragment_shader);
        }
        else if(m_geometry_shader != 0)
        {
            glDeleteShader(m_geometry_shader);
        }
    }

    unsigned int shader::program() const
    {
        return m_program;
    }

    unsigned int shader::uniform_location(const std::string &uniform) const
    {
        return glGetUniformLocation(m_program, uniform.c_str());
    }

    void shader::compile_shaders()
    {
        m_program = glCreateProgram();

        if(m_program == 0)
        {
            utils::prerror("error creating shader program");
        }

        GLint result = 0;
        GLchar error_log[1024] = {0};

        attach_shaders(shader_type::vertex);
        attach_shaders(shader_type::fragment);

        glLinkProgram(m_program);
        glGetProgramiv(m_program, GL_LINK_STATUS, &result);

        if(result == 0)
        {
            glGetProgramInfoLog(m_program, sizeof(error_log), nullptr, error_log);
            utils::prerror("error linking shader program", error_log);
            return;
        }

        glValidateProgram(m_program);
        glGetProgramiv(m_program, GL_VALIDATE_STATUS, &result);

        if(result == 0)
        {
            glGetProgramInfoLog(m_program, sizeof(error_log), nullptr, error_log);
            utils::prerror("error validating shader program", error_log);
            return;
        }
    }

    void shader::attach_shaders(enum shader_type type)
    {
        std::string_view source;
        std::string stype;

        switch (type)
        {
        case shader_type::vertex:
            stype =  "vertex";
            source = m_vertex_source;
            break;
        case shader_type::fragment:
            stype = "fragment";
            source = m_fragment_source;
            break;
        case shader_type::geometry:
            stype = "geometry";
            source = m_geometry_source;
            break;
        }

        GLuint shader = glCreateShader(static_cast<GLuint>(type));
        const GLchar *shader_source[1];
        shader_source[0] = source.data();

        GLint source_len[1];
        source_len[0] = source.length();

        glShaderSource(shader, 1, shader_source, source_len);
        glCompileShader(shader);

        GLint result = 0;
        GLchar error_log[1024] = {0};

        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if(result == 0)
        {
            glGetShaderInfoLog(shader, sizeof(error_log), nullptr, error_log);
            utils::prerror("error compiling " + stype, error_log);
        }

        glAttachShader(m_program, shader);
    }
} // namespace nle
