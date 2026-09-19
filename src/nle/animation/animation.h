/**
 * @file animation.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Keyframe animation clips loaded from a glTF file.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "../core/ref.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>

namespace nle
{

enum class interpolation_type
{
    linear,
    step,
    cubic_spline
};

/// The node property a channel drives.
enum class animation_path
{
    translation,
    rotation,
    scale
};

/**
 * @brief A keyframe track: times in, values out.
 *
 * Values are stored as vec4 whatever the path, so translation and scale use
 * xyz and rotation uses all four components. For cubic_spline there are three
 * values per keyframe -- in-tangent, value, out-tangent -- as glTF stores them.
 */
struct animation_sampler
{
    /// keyframe times in seconds, ascending.
    std::vector<float> input;

    /// keyframe values; input.size() entries, or 3x that for cubic_spline.
    std::vector<glm::vec4> output;

    interpolation_type interpolation = interpolation_type::linear;

    /// Value at @p time, clamped to the first/last keyframe outside the range.
    glm::vec4 sample(float time) const;

    /// As sample(), but interpolating rotations on the unit sphere.
    glm::quat sample_quat(float time) const;

    glm::vec3 sample_vec3(float time) const;

private:
    /// Index of the last keyframe at or before @p time, and the normalised
    /// position of @p time between that keyframe and the next.
    void locate(float time, size_t& key, float& factor) const;
};

struct animation_channel
{
    /// index into skeleton::nodes()
    int node = -1;

    animation_path path = animation_path::translation;

    /// index into animation_clip::samplers
    int sampler = -1;
};

/**
 * @brief One named animation: a set of channels driving a set of nodes.
 */
class animation_clip
{
public:
    animation_clip() = default;
    explicit animation_clip(const std::string& name);

    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    /// Length in seconds: the latest keyframe time of any sampler.
    float duration() const { return m_duration; }

    /// Recomputes duration() from the current samplers.
    void recalculate_duration();

    std::vector<animation_sampler> samplers;
    std::vector<animation_channel> channels;

private:
    std::string m_name;
    float m_duration = 0.0f;
};

} // namespace nle
