#include "animator.h"

#include <algorithm>
#include <cmath>

namespace nle
{

const glm::mat4 animator::IDENTITY = glm::mat4(1.0f);
const std::vector<glm::mat4> animator::NO_JOINTS = {};

animator::animator(ref<class skeleton> skeleton, std::vector<ref<animation_clip>> clips)
    : m_skeleton(skeleton),
      m_clips(std::move(clips))
{
    reset_pose();
    compute_world_matrices();
    compute_joint_matrices();
}

ref<class skeleton> animator::skeleton() const
{
    return m_skeleton;
}

std::vector<std::string> animator::clip_names() const
{
    std::vector<std::string> names;
    names.reserve(m_clips.size());

    for(const auto& clip : m_clips)
    {
        names.push_back(clip ? clip->name() : std::string());
    }

    return names;
}

size_t animator::clip_count() const
{
    return m_clips.size();
}

bool animator::play(const std::string& name, bool loop)
{
    for(size_t i = 0; i < m_clips.size(); ++i)
    {
        if(m_clips[i] && m_clips[i]->name() == name)
        {
            return play(i, loop);
        }
    }

    return false;
}

bool animator::play(size_t index, bool loop)
{
    if(index >= m_clips.size() || !m_clips[index])
    {
        return false;
    }

    m_current_clip = static_cast<int>(index);
    m_looping = loop;
    m_time = 0.0f;
    m_playing = true;

    // pose immediately, so a caller that renders before the next update() sees
    // the first frame of the clip rather than the bind pose.
    update(0.0f);
    return true;
}

void animator::stop()
{
    m_playing = false;
    m_current_clip = -1;
    m_time = 0.0f;

    reset_pose();
    compute_world_matrices();
    compute_joint_matrices();
}

void animator::pause()
{
    m_playing = false;
}

void animator::resume()
{
    if(m_current_clip >= 0)
    {
        m_playing = true;
    }
}

bool animator::playing() const
{
    return m_playing;
}

void animator::set_speed(float speed)
{
    m_speed = speed;
}

float animator::speed() const
{
    return m_speed;
}

void animator::set_looping(bool loop)
{
    m_looping = loop;
}

bool animator::looping() const
{
    return m_looping;
}

float animator::time() const
{
    return m_time;
}

float animator::duration() const
{
    auto clip = current_clip();
    return clip ? clip->duration() : 0.0f;
}

ref<animation_clip> animator::current_clip() const
{
    if(m_current_clip < 0 || static_cast<size_t>(m_current_clip) >= m_clips.size())
    {
        return nullptr;
    }

    return m_clips[static_cast<size_t>(m_current_clip)];
}

void animator::seek(float time)
{
    m_time = time;
    update(0.0f);
}

void animator::update(float delta_time)
{
    auto clip = current_clip();
    if(!clip)
    {
        return;
    }

    if(m_playing)
    {
        m_time += delta_time * m_speed;
    }

    const float length = clip->duration();

    if(length > 0.0f)
    {
        if(m_looping)
        {
            // fmod keeps the sign of the dividend, so negative speeds need a
            // nudge back into [0, length).
            m_time = std::fmod(m_time, length);
            if(m_time < 0.0f)
            {
                m_time += length;
            }
        }
        else if(m_time >= length)
        {
            m_time = length;
            m_playing = false;
        }
        else if(m_time < 0.0f)
        {
            m_time = 0.0f;
            m_playing = false;
        }
    }
    else
    {
        m_time = 0.0f;
    }

    reset_pose();
    apply_clip(m_time);
    compute_world_matrices();
    compute_joint_matrices();
}

void animator::reset_pose()
{
    if(!m_skeleton)
    {
        m_pose.clear();
        return;
    }

    m_pose = m_skeleton->nodes();
}

void animator::apply_clip(float time)
{
    auto clip = current_clip();
    if(!clip)
    {
        return;
    }

    for(const auto& channel : clip->channels)
    {
        if(channel.node < 0 || static_cast<size_t>(channel.node) >= m_pose.size())
        {
            continue;
        }
        if(channel.sampler < 0 || static_cast<size_t>(channel.sampler) >= clip->samplers.size())
        {
            continue;
        }

        const auto& sampler = clip->samplers[static_cast<size_t>(channel.sampler)];
        auto& node = m_pose[static_cast<size_t>(channel.node)];

        switch(channel.path)
        {
            case animation_path::translation:
                node.translation = sampler.sample_vec3(time);
                break;
            case animation_path::rotation:
                node.rotation = sampler.sample_quat(time);
                break;
            case animation_path::scale:
                node.scale = sampler.sample_vec3(time);
                break;
        }
    }
}

void animator::compute_world_matrices()
{
    m_world_matrices.assign(m_pose.size(), glm::mat4(1.0f));

    if(!m_skeleton)
    {
        return;
    }

    // Parents are not guaranteed to come before their children in the file, so
    // descend from the roots rather than sweeping the array in order.
    std::vector<int> stack(m_skeleton->roots().begin(), m_skeleton->roots().end());

    while(!stack.empty())
    {
        const int index = stack.back();
        stack.pop_back();

        if(index < 0 || static_cast<size_t>(index) >= m_pose.size())
        {
            continue;
        }

        const auto& node = m_pose[static_cast<size_t>(index)];
        const glm::mat4 parent = node.parent >= 0 && static_cast<size_t>(node.parent) < m_world_matrices.size()
            ? m_world_matrices[static_cast<size_t>(node.parent)]
            : glm::mat4(1.0f);

        m_world_matrices[static_cast<size_t>(index)] = parent * node.local_matrix();

        for(int child : node.children)
        {
            stack.push_back(child);
        }
    }
}

void animator::compute_joint_matrices()
{
    if(!m_skeleton)
    {
        m_joint_matrices.clear();
        return;
    }

    const auto& skins = m_skeleton->skins();
    m_joint_matrices.resize(skins.size());

    for(size_t s = 0; s < skins.size(); ++s)
    {
        const auto& skin = skins[s];
        auto& palette = m_joint_matrices[s];
        palette.assign(skin.joints.size(), glm::mat4(1.0f));

        for(size_t j = 0; j < skin.joints.size(); ++j)
        {
            const int node = skin.joints[j];
            if(node < 0 || static_cast<size_t>(node) >= m_world_matrices.size())
            {
                continue;
            }

            const glm::mat4 inverse_bind = j < skin.inverse_bind_matrices.size()
                ? skin.inverse_bind_matrices[j]
                : glm::mat4(1.0f);

            // The mesh node's own transform is deliberately absent: glTF says a
            // skinned mesh is posed purely by its joints, in the skin's space.
            palette[j] = m_world_matrices[static_cast<size_t>(node)] * inverse_bind;
        }
    }
}

const glm::mat4& animator::node_world_matrix(int node) const
{
    if(node < 0 || static_cast<size_t>(node) >= m_world_matrices.size())
    {
        return IDENTITY;
    }

    return m_world_matrices[static_cast<size_t>(node)];
}

const std::vector<glm::mat4>& animator::joint_matrices(size_t skin_index) const
{
    if(skin_index >= m_joint_matrices.size())
    {
        return NO_JOINTS;
    }

    return m_joint_matrices[skin_index];
}

} // namespace nle
