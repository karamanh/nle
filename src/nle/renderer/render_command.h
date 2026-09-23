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
#include "render_context.h"
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
    draw_elements_instanced,
    set_depth_mask,
    set_blending,

    /// Which lamps reach this surface. Per draw rather than per frame,
    /// because which eight lamps matter depends on where the surface is and
    /// not on where the camera happens to be standing.
    set_point_lights
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

    /// How many copies. One is an ordinary draw; more is instancing, where
    /// the shader is handed gl_InstanceID and works out the rest itself.
    size_t instances = 1;
};

/**
 * @brief How what is being drawn is combined with what is already there.
 *
 * Only the two that matter so far. Opaque geometry writes over the frame;
 * a glow adds to it, which is what makes a hundred faint sparks read as one
 * bright one where they overlap.
 */
enum class blend_mode
{
    none,
    additive,
    alpha
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
        draw_elements_data,        // draw_elements, draw_elements_instanced
        bool,                      // set_depth_mask
        blend_mode,                // set_blending
        std::vector<point_light_data>  // set_point_lights
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

    /**
     * @brief The lamps that reach the surface about to be drawn.
     *
     * Recorded per draw. The backend drops it when the set is the one already
     * uploaded, which it usually is: everything standing in the same part of
     * a town is reached by the same lamps.
     */
    void set_point_lights(std::vector<point_light_data> lights);
    void use_texture(ref<texture> texture, unsigned int unit = 0);
    void draw_elements(unsigned int primitive_type, unsigned int vao, unsigned int ebo, size_t count);

    /**
     * @brief The same draw, @p instances times over.
     *
     * For anything there are a great many of that differ only in numbers the
     * shader can work out for itself -- particles, grass, a crowd. One call,
     * one buffer, no per-copy work on this side.
     */
    void draw_elements_instanced(unsigned int primitive_type, unsigned int vao,
                                 unsigned int ebo, size_t count, size_t instances);

    void set_depth_mask(bool mask);
    void set_blending(blend_mode mode);

    void clear();
    const std::vector<render_command>& commands() const;

private:
    std::vector<render_command> m_commands;
};

} // namespace nle
