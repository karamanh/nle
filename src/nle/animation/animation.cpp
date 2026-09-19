#include "animation.h"

#include <algorithm>

namespace nle
{

namespace
{
    /// glTF cubic spline interpolation, per the specification's formula.
    glm::vec4 cubic_spline(const glm::vec4& value_a, const glm::vec4& out_tangent_a,
                           const glm::vec4& value_b, const glm::vec4& in_tangent_b,
                           float delta, float t)
    {
        const float t2 = t * t;
        const float t3 = t2 * t;

        return (2.0f * t3 - 3.0f * t2 + 1.0f) * value_a
             + delta * (t3 - 2.0f * t2 + t) * out_tangent_a
             + (-2.0f * t3 + 3.0f * t2) * value_b
             + delta * (t3 - t2) * in_tangent_b;
    }
}

void animation_sampler::locate(float time, size_t& key, float& factor) const
{
    key = 0;
    factor = 0.0f;

    if(input.size() < 2)
    {
        return;
    }

    if(time <= input.front())
    {
        return;
    }

    if(time >= input.back())
    {
        key = input.size() - 2;
        factor = 1.0f;
        return;
    }

    // first keyframe strictly after `time`, so the one before it brackets it.
    const auto upper = std::upper_bound(input.begin(), input.end(), time);
    key = static_cast<size_t>(std::distance(input.begin(), upper)) - 1;

    const float span = input[key + 1] - input[key];
    factor = span > 0.0f ? (time - input[key]) / span : 0.0f;
}

glm::vec4 animation_sampler::sample(float time) const
{
    if(output.empty())
    {
        return glm::vec4(0.0f);
    }

    if(input.size() < 2)
    {
        return interpolation == interpolation_type::cubic_spline ? output[std::min<size_t>(1, output.size() - 1)]
                                                                 : output.front();
    }

    size_t key = 0;
    float factor = 0.0f;
    locate(time, key, factor);

    switch(interpolation)
    {
        case interpolation_type::step:
        {
            const size_t index = factor >= 1.0f ? key + 1 : key;
            return output[std::min(index, output.size() - 1)];
        }
        case interpolation_type::cubic_spline:
        {
            // three values per keyframe: in-tangent, value, out-tangent.
            const size_t base_a = key * 3;
            const size_t base_b = (key + 1) * 3;

            if(base_b + 1 >= output.size())
            {
                return output[std::min(base_a + 1, output.size() - 1)];
            }

            const float delta = input[key + 1] - input[key];
            return cubic_spline(output[base_a + 1], output[base_a + 2],
                                output[base_b + 1], output[base_b],
                                delta, factor);
        }
        case interpolation_type::linear:
        default:
        {
            if(key + 1 >= output.size())
            {
                return output.back();
            }
            return glm::mix(output[key], output[key + 1], factor);
        }
    }
}

glm::quat animation_sampler::sample_quat(float time) const
{
    if(output.empty())
    {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    // Rotations must travel along the sphere, so linear keyframes are slerped
    // rather than componentwise mixed. Cubic spline is evaluated as the spec
    // says and then renormalised.
    if(interpolation == interpolation_type::linear && input.size() >= 2)
    {
        size_t key = 0;
        float factor = 0.0f;
        locate(time, key, factor);

        if(key + 1 < output.size())
        {
            const glm::quat a(output[key].w, output[key].x, output[key].y, output[key].z);
            const glm::quat b(output[key + 1].w, output[key + 1].x, output[key + 1].y, output[key + 1].z);
            return glm::normalize(glm::slerp(a, b, factor));
        }
    }

    const glm::vec4 value = sample(time);
    return glm::normalize(glm::quat(value.w, value.x, value.y, value.z));
}

glm::vec3 animation_sampler::sample_vec3(float time) const
{
    return glm::vec3(sample(time));
}

animation_clip::animation_clip(const std::string& name)
    : m_name(name)
{
}

void animation_clip::recalculate_duration()
{
    m_duration = 0.0f;

    for(const auto& sampler : samplers)
    {
        if(!sampler.input.empty())
        {
            m_duration = std::max(m_duration, sampler.input.back());
        }
    }
}

} // namespace nle
