/**
 * @file render_command.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Render command system for graphics API abstraction
 * @version 0.1
 * @date 2024-11-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "../core/ref.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <variant>

namespace nle
{

// Forward declarations
class shader;
class texture;
class material;

enum class command_type
{
    set_polygon_mode,
    use_shader,
    set_uniform_matrix4,
    set_uniform_matrix4_array,
    set_uniform_vec3,
    set_uniform_vec4,
    set_uniform_float,
    set_uniform_int,
    use_texture,
    draw_elements,
    set_depth_mask
};

struct uniform_data
{
    std::string name;
    std::variant<glm::mat4, glm::vec3, glm::vec4, float, int, std::vector<glm::mat4>> value;
};

struct polygon_mode_data
{
    unsigned int front_and_back;
    unsigned int mode;
};

struct texture_bind_data
{
    /// not named `texture`: that would shadow the class name inside the struct.
    ref<class texture> handle;
    unsigned int unit;
};

struct draw_elements_data
{
    unsigned int primitive_type;
    unsigned int vao;
    unsigned int ebo;
    size_t count;
};

struct render_command
{
    command_type type;
    
    // Command-specific data
    std::variant<
        polygon_mode_data,         // set_polygon_mode
        ref<shader>,               // use_shader
        uniform_data,              // uniform data
        texture_bind_data,         // use_texture
        draw_elements_data,        // draw_elements
        bool                       // set_depth_mask
    > data;
};

class render_command_buffer
{
public:
    render_command_buffer();
    ~render_command_buffer();

    void set_polygon_mode(unsigned int front_and_back, unsigned int mode);
    void use_shader(ref<shader> shader);
    void set_uniform(const std::string& name, const glm::mat4& value);
    void set_uniform(const std::string& name, const std::vector<glm::mat4>& value);
    void set_uniform(const std::string& name, const glm::vec3& value);
    void set_uniform(const std::string& name, const glm::vec4& value);
    void set_uniform(const std::string& name, float value);
    void set_uniform(const std::string& name, int value);
    void use_texture(ref<texture> texture, unsigned int unit = 0);
    void draw_elements(unsigned int primitive_type, unsigned int vao, unsigned int ebo, size_t count);
    void set_depth_mask(bool mask);

    void clear();
    const std::vector<render_command>& commands() const;

private:
    std::vector<render_command> m_commands;
};

} // namespace nle
