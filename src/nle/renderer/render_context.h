/**
 * @file render_context.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Per-frame state shared by every draw in a frame.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace nle
{

/// Upper bound on the point lights uploaded in a single frame. Must match
/// NLE_MAX_POINT_LIGHTS in the shaders.
constexpr int MAX_POINT_LIGHTS = 8;

/// Upper bound on the joints of a single skin. Must match NLE_MAX_JOINTS in
/// the shaders. Kept conservative: 64 joints is 1024 uniform components, which
/// every GL 3.3 implementation is required to provide.
constexpr int MAX_JOINTS = 64;

struct directional_light_data
{
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec3 ambient = glm::vec3(1.0f);
    glm::vec3 diffuse = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(1.0f);
    bool enabled = true;
};

struct point_light_data
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec3 ambient = glm::vec3(0.0f);
    glm::vec3 diffuse = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(1.0f);

    /// attenuation = 1 / (constant + linear * d + quadratic * d * d)
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    /// hard cutoff, beyond which the light contributes nothing.
    float range = 50.0f;
};

struct fog_data
{
    bool enabled = false;
    float near_distance = 0.0f;
    float far_distance = 1000.0f;
    /// colour fragments fade towards at far_distance.
    glm::vec3 color = glm::vec3(1.0f);
};

/**
 * @brief Everything a draw needs that is constant for the whole frame.
 *
 * Built once per frame by renderer_3d and uploaded once per shader program by
 * opengl_backend, rather than re-emitted by every render object.
 */
struct render_context
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::vec3 eye_position = glm::vec3(0.0f);
    glm::vec2 resolution = glm::vec2(1.0f);

    /// seconds since the renderer started, and since the previous frame.
    float time = 0.0f;
    float delta_time = 0.0f;

    directional_light_data directional_light;
    std::vector<point_light_data> point_lights;
    fog_data fog;
};

} // namespace nle
