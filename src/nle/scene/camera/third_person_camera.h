/**
 * @file third_person_camera.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief An orbiting follow camera on a spring arm.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "../camera.h"
#include "../terrain_3d.h"

#include <functional>

namespace nle
{

/**
 * @brief A camera that orbits a point and is pushed in by whatever it hits.
 *
 * The camera sits at the end of an arm anchored at the target: yaw and pitch
 * aim the arm, distance sets how long it wants to be, and the arm is shortened
 * whenever something would come between the camera and what it is watching.
 * That is the whole model -- there is no separate collision volume, and the
 * camera never ends up inside the ground.
 *
 *     auto view = make_ref<third_person_camera>();
 *     view->set_terrain(ground);
 *     view->set_distance(16.0f);
 *     scene->set_camera(view);
 *
 *     // per frame
 *     view->set_target(hero->position());
 *     view->orbit(mouse_delta_x, mouse_delta_y);
 *     view->update(delta_time);
 *
 * The arm collapses immediately when something blocks it and eases back out
 * once the way is clear, which is what keeps a camera from stuttering as it
 * clips a corner. set_return_speed() controls only the easing out.
 */
class third_person_camera : public camera
{
public:
    /**
     * @brief Reports what the arm runs into.
     *
     * @p arm starts at the pivot and points at where the camera wants to be.
     * Write the distance along it of the nearest obstruction to @p blocked_at
     * and return true; return false when nothing is in the way.
     *
     * Terrain is handled separately by set_terrain(); this is for everything
     * else the camera should not pass through.
     */
    using collision_probe = std::function<bool(const ray& arm, float length, float& blocked_at)>;

    third_person_camera();
    virtual ~third_person_camera();

    /// The point the camera orbits and looks at, usually a character's feet.
    void set_target(const glm::vec3& target);
    glm::vec3 target() const;

    /**
     * @brief How far above the target the arm is anchored.
     *
     * Anchoring at the feet and looking at the feet puts the character at the
     * centre of the screen with the ground filling the lower half, so the
     * pivot is normally lifted to about chest height.
     */
    void set_pivot_height(float height);
    float pivot_height() const;

    /// Where the arm points, in degrees. Yaw wraps; pitch is clamped.
    void set_yaw(float yaw);
    float yaw() const;

    void set_pitch(float pitch);
    float pitch() const;

    /**
     * @brief Clamps the pitch, in degrees.
     *
     * Defaults to +/- 80. The poles are what the limits are really for: an arm
     * pointing straight up or straight down has no well defined yaw, so the
     * view rolls over as it passes through, and stopping short of them is what
     * keeps the camera from behaving strangely overhead.
     */
    void set_pitch_limits(float minimum, float maximum);
    float minimum_pitch() const;
    float maximum_pitch() const;

    /**
     * @brief Turns the camera by a mouse movement, in screen pixels.
     *
     * Both deltas are in the same frame the cursor position is reported in:
     * x grows to the right, y grows downwards. Feed it the difference between
     * two readings of mouse_x()/mouse_y() and nothing needs negating.
     *
     * The sense is mouse look: dragging right turns the view right, dragging
     * up looks up. Turning the view right swings the camera itself to the
     * left, which is what an arm anchored on the target has to do.
     */
    void orbit(float delta_x, float delta_y);

    /**
     * @brief Flips the vertical sense, so dragging up looks down.
     *
     * The horizontal sense is not negotiable -- dragging right must turn the
     * view right -- but which way is "up" is a preference, and a game with a
     * mouse usually offers it.
     */
    void set_invert_pitch(bool invert);
    bool invert_pitch() const;

    /// Degrees turned per pixel of mouse movement.
    void set_sensitivity(float degrees_per_pixel);
    float sensitivity() const;

    /// How long the arm is when nothing is in the way.
    void set_distance(float distance);
    float distance() const;

    /// Shortest the arm is allowed to be collapsed to.
    void set_minimum_distance(float distance);

    /// What the arm has actually settled at, after collisions.
    float arm_length() const;

    /// How fast, in world units per second, the arm eases back out.
    void set_return_speed(float units_per_second);

    /// Ground the camera must stay above. Optional.
    void set_terrain(ref<class terrain_3d> terrain);

    /// How far above the terrain the camera is kept.
    void set_ground_clearance(float clearance);
    float ground_clearance() const;

    /// Anything besides the terrain the arm should not pass through.
    void set_collision_probe(collision_probe probe);

    /// Advances the spring and writes the camera's position and rotation.
    void update(float delta_time);

    /// Puts the camera at its resting pose at once, skipping the spring.
    void snap();

protected:
    /// update(float) would otherwise hide object_3d's no-argument update(),
    /// which is the internal recompute of the front/right/up basis and is not
    /// the same thing at all.
    using object_3d::update;

private:
    glm::vec3 m_target = glm::vec3(0.0f);
    float m_pivot_height = 1.0f;

    float m_yaw = 0.0f;
    float m_pitch = 35.0f;
    float m_minimum_pitch = -80.0f;
    float m_maximum_pitch = 80.0f;

    float m_sensitivity = 0.25f;
    bool m_invert_pitch = false;

    float m_distance = 16.0f;
    float m_minimum_distance = 1.5f;
    float m_arm_length = 16.0f;
    float m_return_speed = 12.0f;

    ref<class terrain_3d> m_terrain;
    float m_ground_clearance = 0.6f;

    collision_probe m_probe;

    /// Where the arm is anchored: the target, lifted.
    glm::vec3 pivot() const;

    /// Unit vector from the pivot towards where the camera wants to be.
    glm::vec3 arm_direction() const;

    /// Longest the arm can be before it meets the ground or an obstruction.
    float unobstructed_length(const glm::vec3& origin, const glm::vec3& direction) const;

    /// Points the camera back at the pivot from @p position.
    void look_at_pivot(const glm::vec3& position);
};

} // namespace nle
