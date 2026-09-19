/**
 * @file renderer_2d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Screen-space overlay rendering: health bars, skill bars, and so on.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "../core/ref.h"
#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace nle
{

struct vertex_2d
{
    glm::vec2 position;
    glm::vec2 uv;
    glm::vec4 color;
};

/**
 * @brief Draws flat, screen-space geometry on top of the 3D scene.
 *
 * Coordinates are in pixels with the origin at the top left, so a bar at
 * (20, 20) is twenty pixels in from the top left corner whatever the window
 * size. Call begin() with the current resolution, issue draws, then end().
 *
 * Quads are batched into one buffer and flushed together. Untextured draws
 * sample a 1x1 white texture, which means a whole HUD of bars and panels
 * costs a single draw call.
 *
 * This deliberately does not go through render_command_buffer: the 2D pass
 * rewrites its vertex buffer every frame, which the command buffer has no
 * vocabulary for, and it owns a small, self-contained piece of GL state.
 * It must run after the 3D pass -- window::render_ui() is the place.
 */
class renderer_2d
{
public:
    /// Loads the overlay shader from disk.
    renderer_2d(const std::string& vertex_shader_path, const std::string& fragment_shader_path);

    /// Uses an already-compiled overlay shader.
    explicit renderer_2d(ref<class shader> shader);

    ~renderer_2d();

    /**
     * @brief Starts a frame of overlay drawing.
     *
     * Disables depth testing and enables alpha blending for the duration;
     * end() puts both back.
     */
    void begin(const glm::vec2& resolution);
    void end();

    void draw_rect(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

    /// A rectangular frame drawn just inside the given bounds.
    void draw_outline(const glm::vec2& position, const glm::vec2& size, float thickness,
                      const glm::vec4& color);

    void draw_texture(const glm::vec2& position, const glm::vec2& size, ref<class texture> texture,
                      const glm::vec4& tint = glm::vec4(1.0f),
                      const glm::vec2& uv_min = glm::vec2(0.0f),
                      const glm::vec2& uv_max = glm::vec2(1.0f));

    /**
     * @brief A bar that fills from the left.
     *
     * @param fill fraction in [0, 1]; clamped.
     */
    void draw_bar(const glm::vec2& position, const glm::vec2& size, float fill,
                  const glm::vec4& fill_color,
                  const glm::vec4& background_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f),
                  const glm::vec4& border_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.85f),
                  float border = 1.0f);

    /**
     * @brief A cooldown sweep: shades the part of a slot that is still on
     *        cooldown, filling from the bottom up as it recovers.
     *
     * @param remaining fraction of the cooldown left, in [0, 1].
     */
    void draw_cooldown_overlay(const glm::vec2& position, const glm::vec2& size, float remaining,
                               const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 0.65f));

    glm::vec2 resolution() const;

    /// Draw calls issued during the last completed frame.
    size_t draw_calls() const;

private:
    void create_buffers();
    void create_white_texture();

    void push_quad(const glm::vec2& position, const glm::vec2& size,
                   const glm::vec2& uv_min, const glm::vec2& uv_max,
                   const glm::vec4& color);

    /// Switching texture ends the current batch.
    void use_texture(const ref<class texture>& texture);

    void flush();

    ref<class shader> m_shader;
    ref<class texture> m_white_texture;
    ref<class texture> m_current_texture;

    std::vector<vertex_2d> m_vertices;

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;

    size_t m_vbo_capacity_quads = 0;

    glm::vec2 m_resolution = glm::vec2(1.0f);
    bool m_in_frame = false;

    size_t m_draw_calls = 0;
    size_t m_frame_draw_calls = 0;
};

} // namespace nle
