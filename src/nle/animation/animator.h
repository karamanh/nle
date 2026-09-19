/**
 * @file animator.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Plays one animation clip over a skeleton.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "animation.h"
#include "skeleton.h"

namespace nle
{

/**
 * @brief Plays a single clip over a per-instance copy of a skeleton's pose.
 *
 * Deliberately simple: one clip at a time, no blending and no layering. Each
 * update() samples the clip into the local pose, walks the hierarchy to get
 * world matrices, and from those builds one joint matrix palette per skin,
 * ready to be handed to the vertex shader.
 *
 * The skeleton is shared between instances; the pose is not, so several
 * instances of the same model can play different clips at different times.
 */
class animator
{
public:
    animator(ref<class skeleton> skeleton, std::vector<ref<animation_clip>> clips);

    std::vector<std::string> clip_names() const;
    size_t clip_count() const;

    /// Starts a clip by name. Returns false (and changes nothing) if no clip
    /// with that name exists.
    bool play(const std::string& name, bool loop = true);
    bool play(size_t index, bool loop = true);

    /// Stops playback and returns the skeleton to its bind pose.
    void stop();

    void pause();
    void resume();
    bool playing() const;

    /// Playback rate. 1.0 is the clip's authored speed; negative plays back.
    void set_speed(float speed);
    float speed() const;

    void set_looping(bool loop);
    bool looping() const;

    /**
     * @brief Jumps to @p time (seconds) within the current clip and re-poses.
     *
     * While looping, @p time wraps into [0, duration), so seeking to exactly
     * the duration lands on the first frame -- for a loop, that is the same
     * pose. Turn looping off to reach the final frame.
     */
    void seek(float time);
    float time() const;
    float duration() const;

    ref<animation_clip> current_clip() const;

    /// Advances playback by @p delta_time seconds and recomputes the pose.
    void update(float delta_time);

    /// World matrix of a node under the current pose.
    const glm::mat4& node_world_matrix(int node) const;

    /**
     * @brief Joint palette for one skin: joint_world * inverse_bind per joint.
     *
     * This is exactly what the vertex shader's u_joint_matrices wants. Returns
     * an empty vector for an unknown skin index.
     */
    const std::vector<glm::mat4>& joint_matrices(size_t skin_index) const;

    ref<class skeleton> skeleton() const;

private:
    /// Resets the local pose to the skeleton's bind transforms.
    void reset_pose();

    /// Writes the current clip's value at @p time into the local pose.
    void apply_clip(float time);

    void compute_world_matrices();
    void compute_joint_matrices();

    ref<class skeleton> m_skeleton;
    std::vector<ref<animation_clip>> m_clips;

    /// per-instance, animated copy of the skeleton's nodes.
    std::vector<skeleton_node> m_pose;
    std::vector<glm::mat4> m_world_matrices;
    std::vector<std::vector<glm::mat4>> m_joint_matrices;

    int m_current_clip = -1;
    float m_time = 0.0f;
    float m_speed = 1.0f;
    bool m_playing = false;
    bool m_looping = true;

    static const glm::mat4 IDENTITY;
    static const std::vector<glm::mat4> NO_JOINTS;
};

} // namespace nle
