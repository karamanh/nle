#ifndef NLE_MODEL_H
#define NLE_MODEL_H

#include "Nle.h"
#include "Mesh.h"
#include "Texture.h"

namespace nle
{
    enum class ModelFormat
    {
        OBJ,
        GLTF
    };
    class Model
    {
    private:
        MultiMesh *m_multimesh;
        Shader *m_shader = nullptr;
        Texture *m_texture = nullptr;
        std::vector<Texture*> m_textures;

    public:
        Model(const std::string& path, Shader *shader, Texture *texture = nullptr);
        MultiMeshInstance *create_instance();
        bool load_obj(const std::string &path);
        bool load_gltf(const std::string &path);
        MultiMesh *multimesh() const;
    };
} // namespace nle

#endif