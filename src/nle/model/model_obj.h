/**
 * @file model_obj.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief
 * @version 0.1
 * @date 2024-02-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "model.hpp"

namespace nle
{

    /**
     * @brief A Wavefront .obj model, with its .mtl materials.
     *
     * Geometry only: no rig and no animation, which is what separates it from
     * model_gltf. Scenery, in other words -- buildings, rocks, furniture --
     * where glTF is for anything that has to move under its own power.
     *
     * Materials come from the .mtl the file names: ambient, diffuse, specular,
     * shininess and dissolve, plus map_Kd, which is looked for next to the
     * model when the path in the .mtl is relative.
     */
    class model_obj : public model
    {
    public:
        /// @throws std::runtime_error if the file is not a .obj or will not load.
        model_obj(const std::string& path);
        virtual ~model_obj();

        ref<multimesh_instance_3d> create_instance();

        std::string name() const;
    private:
        void load(const std::string &path);
    };

} // namespace nle
