/**
 * @file box_mesh.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief
 * @version 0.1
 * @date 2024-08-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <vector>
#include "mesh_3d.h"
#include "../renderer/vertex.h"

namespace nle
{
    class boxmesh : public mesh_3d
    {
    public:
        boxmesh();

    private:
        const static std::vector<vertex> VERTICES;
        const static std::vector<unsigned int> INDICES;
    };

} // namespace nle
