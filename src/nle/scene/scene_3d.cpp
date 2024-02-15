#include "scene_3d.h"

namespace nle
{

scene_3d::scene_3d()
{
    m_default_camera = make_ref<class camera>();
}

scene_3d::~scene_3d()
{
}

void scene_3d::set_camera(ref<class camera> camera)
{
    m_camera = camera;
}

ref<class camera> scene_3d::camera()
{
    return m_camera == nullptr ? m_default_camera : m_camera;
}

ref<class camera> scene_3d::default_camera()
{
    return m_default_camera;
}

void scene_3d::add_child(ref<object_3d> child)
{
    object_3d::add_child(child);
    
    /// register as render object (if it's a render object)
    auto ro = std::dynamic_pointer_cast<render_object_3d>(child);
    if(ro)
    {
        m_render_objects.insert(ro);
    }
}

void scene_3d::delete_child(ref<object_3d> child)
{
    /// first we unregister render object (if it's a render object)
    auto ro = std::dynamic_pointer_cast<render_object_3d>(child);
    if(ro)
    {
        m_render_objects.erase(ro);
    }

    object_3d::delete_child(child);
}

nlohmann::json scene_3d::to_json() const
{
    auto j = object_3d::to_json();
    throw std::runtime_error("nlohmann::json scene_3d::to_json() not fully implemented");
    return j;
}

void scene_3d::from_json(const nlohmann::json &j)
{
    object_3d::from_json(j);
    throw std::runtime_error("nlohmann::json scene_3d::from_json() not fully implemented");
}

} // namespace nle
