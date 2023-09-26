#pragma once

#include "MultiMesh.h"

namespace nle
{
    class Sky : public MeshInstance
    {
    private:
        bool m_distance_fog_enabled;
        float m_distance_fog_min;
        float m_distance_fog_max;
        Mesh *m_mesh = nullptr;
    public:
        Sky();
        Sky(Mesh *m);
        ~Sky();

        void set_distance_fog_enabled(bool enabled);
        bool distance_fog_enabled() const;

        void set_distance_fog_max(float max);
        float distance_fog_max() const;

        void set_distance_fog_min(float min);
        float distance_fog_min() const;
    };
} // namespace nle
