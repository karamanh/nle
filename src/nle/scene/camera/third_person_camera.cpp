#include "third_person_camera.h"

#include <algorithm>
#include <cmath>

namespace nle
{

namespace
{
    /// Step used when walking the arm across the terrain, and the bisection
    /// passes used to tighten the contact afterwards. The march only has to
    /// cover the arm's own length, so it can afford to be fine.
    constexpr float ARM_STEP = 0.25f;
    constexpr int ARM_BISECTION_STEPS = 12;
}

third_person_camera::third_person_camera()
{
    snap();
}

third_person_camera::~third_person_camera()
{
}

void third_person_camera::set_target(const glm::vec3& target)
{
    m_target = target;
}

glm::vec3 third_person_camera::target() const
{
    return m_target;
}

void third_person_camera::set_pivot_height(float height)
{
    m_pivot_height = height;
}

float third_person_camera::pivot_height() const
{
    return m_pivot_height;
}

void third_person_camera::set_yaw(float yaw)
{
    m_yaw = std::fmod(std::fmod(yaw, 360.0f) + 360.0f, 360.0f);
}

float third_person_camera::yaw() const
{
    return m_yaw;
}

void third_person_camera::set_pitch(float pitch)
{
    m_pitch = std::clamp(pitch, m_minimum_pitch, m_maximum_pitch);
}

float third_person_camera::pitch() const
{
    return m_pitch;
}

void third_person_camera::set_pitch_limits(float minimum, float maximum)
{
    m_minimum_pitch = std::min(minimum, maximum);
    m_maximum_pitch = std::max(minimum, maximum);

    // Never leave the current pitch outside the limits just set.
    set_pitch(m_pitch);
}

float third_person_camera::minimum_pitch() const
{
    return m_minimum_pitch;
}

float third_person_camera::maximum_pitch() const
{
    return m_maximum_pitch;
}

void third_person_camera::orbit(float delta_x, float delta_y)
{
    // Both signs are negative against the raw screen delta, for the same
    // reason: the arm points from the target at the camera, which is the
    // opposite of where the camera is looking. Adding the delta would turn
    // the arm the way the mouse went and therefore swing the view the other
    // way, which reads as inverted on both axes.
    set_yaw(m_yaw - delta_x * m_sensitivity);

    // Screen y already grows downwards, so dragging up is a negative delta
    // and lowers the arm, which raises the view.
    set_pitch(m_pitch + (m_invert_pitch ? -delta_y : delta_y) * m_sensitivity);
}

void third_person_camera::set_invert_pitch(bool invert)
{
    m_invert_pitch = invert;
}

bool third_person_camera::invert_pitch() const
{
    return m_invert_pitch;
}

void third_person_camera::set_sensitivity(float degrees_per_pixel)
{
    m_sensitivity = degrees_per_pixel;
}

float third_person_camera::sensitivity() const
{
    return m_sensitivity;
}

void third_person_camera::set_distance(float distance)
{
    m_distance = std::max(distance, m_minimum_distance);
}

float third_person_camera::distance() const
{
    return m_distance;
}

void third_person_camera::set_minimum_distance(float distance)
{
    m_minimum_distance = std::max(distance, 0.0f);
    m_distance = std::max(m_distance, m_minimum_distance);
}

float third_person_camera::arm_length() const
{
    return m_arm_length;
}

void third_person_camera::set_return_speed(float units_per_second)
{
    m_return_speed = std::max(units_per_second, 0.0f);
}

void third_person_camera::set_terrain(ref<class terrain_3d> terrain)
{
    m_terrain = terrain;
}

void third_person_camera::set_ground_clearance(float clearance)
{
    m_ground_clearance = std::max(clearance, 0.0f);
}

float third_person_camera::ground_clearance() const
{
    return m_ground_clearance;
}

void third_person_camera::set_collision_probe(collision_probe probe)
{
    m_probe = std::move(probe);
}

glm::vec3 third_person_camera::pivot() const
{
    return m_target + glm::vec3(0.0f, m_pivot_height, 0.0f);
}

glm::vec3 third_person_camera::arm_direction() const
{
    const float yaw = glm::radians(m_yaw);
    const float pitch = glm::radians(m_pitch);
    const float flat = std::cos(pitch);

    // Yaw of zero puts the camera on +Z, which is behind a model that faces
    // +Z in its bind pose. Positive pitch lifts it.
    return { flat * std::sin(yaw), std::sin(pitch), flat * std::cos(yaw) };
}

float third_person_camera::unobstructed_length(const glm::vec3& origin, const glm::vec3& direction) const
{
    float limit = m_distance;

    if(m_terrain)
    {
        // Walk the arm and stop where it first drops within clearance of the
        // ground. The terrain is a heightfield, so "is this point underground"
        // is a single lookup and marching it needs no acceleration structure.
        auto below_ground = [&](float along) {
            const glm::vec3 point = origin + direction * along;
            return point.y < m_terrain->height_at(point.x, point.z) + m_ground_clearance;
        };

        if(below_ground(0.0f))
        {
            // The pivot itself is under the ground, so there is no length that
            // helps. The vertical clamp in update() is what recovers this.
            limit = m_minimum_distance;
        }
        else
        {
            float last_clear = 0.0f;

            for(float along = ARM_STEP; along <= limit; along += ARM_STEP)
            {
                if(below_ground(along))
                {
                    // Tighten the contact so the arm does not visibly step in
                    // and out by ARM_STEP as the ground slides past.
                    float low = last_clear;
                    float high = along;

                    for(int step = 0; step < ARM_BISECTION_STEPS; ++step)
                    {
                        const float middle = (low + high) * 0.5f;

                        if(below_ground(middle))
                        {
                            high = middle;
                        }
                        else
                        {
                            low = middle;
                        }
                    }

                    limit = low;
                    break;
                }

                last_clear = along;
            }
        }
    }

    if(m_probe)
    {
        float blocked_at = limit;

        if(m_probe(ray{ origin, direction }, limit, blocked_at))
        {
            limit = std::min(limit, blocked_at);
        }
    }

    return std::clamp(limit, m_minimum_distance, m_distance);
}

void third_person_camera::update(float delta_time)
{
    const glm::vec3 anchor = pivot();
    const glm::vec3 direction = arm_direction();

    const float wanted = unobstructed_length(anchor, direction);

    // Collapse at once so the camera never ends up inside anything, and ease
    // back out, so clipping a corner does not throw the view around.
    if(wanted < m_arm_length)
    {
        m_arm_length = wanted;
    }
    else
    {
        m_arm_length = std::min(wanted, m_arm_length + m_return_speed * delta_time);
    }

    glm::vec3 placed = anchor + direction * m_arm_length;

    // Last resort: a pivot that is itself underground, or ground that rises
    // faster than the arm test sampled, still must not swallow the camera.
    if(m_terrain)
    {
        placed.y = std::max(placed.y, m_terrain->height_at(placed.x, placed.z) + m_ground_clearance);
    }

    look_at_pivot(placed);
}

void third_person_camera::look_at_pivot(const glm::vec3& placed)
{
    const glm::vec3 anchor = pivot();
    const glm::vec3 to_pivot = anchor - placed;

    set_position(placed);

    if(glm::length(to_pivot) < 1e-5f)
    {
        return;
    }

    const glm::vec3 forward = glm::normalize(to_pivot);

    // The euler angles the base camera turns into a view matrix: pitch from
    // the rise of the look direction, yaw about Y.
    const float pitch = glm::degrees(std::asin(std::clamp(forward.y, -1.0f, 1.0f)));
    const float yaw = glm::degrees(std::atan2(-forward.x, -forward.z));

    set_rotation({ pitch, yaw, 0.0f });
}

void third_person_camera::snap()
{
    const glm::vec3 anchor = pivot();
    const glm::vec3 direction = arm_direction();

    m_arm_length = unobstructed_length(anchor, direction);

    glm::vec3 placed = anchor + direction * m_arm_length;

    if(m_terrain)
    {
        placed.y = std::max(placed.y, m_terrain->height_at(placed.x, placed.z) + m_ground_clearance);
    }

    look_at_pivot(placed);
}

} // namespace nle
