#include "TextureFactory.h"

namespace nle
{

    TextureFactory::TextureFactory()
    {
    }

    TextureFactory::~TextureFactory()
    {
        for(auto it = m_textures.begin(); it != m_textures.end(); ++it)
        {
            delete it->second;
        }
    }

    bool TextureFactory::load(const std::string &path, const std::string &id)
    {
        if (m_textures.find(path) != m_textures.end())
        {
            prerr("texture with same id already exists: %s", id.c_str());
            return false;
        }

        Texture *t = new Texture(path);
        if (t->id() > 0)
        {
            m_textures[id] = t;
            return true;
        }

        delete t;
        return false;
    }

    Texture *TextureFactory::get(const std::string &id)
    {
        return m_textures[id];
    }

    Texture *TextureFactory::load_and_get(const std::string &path, const std::string &id)
    {
        if(load(path, id))
        {
            return get(id);
        }
        return nullptr;
    }

} // namespace nle
