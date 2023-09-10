#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace nle
{
    Texture::Texture(const std::string &file_path)
    {
        clear();
        load_from_file((m_file_path = file_path));
    }

    Texture::~Texture()
    {
        clear();
    }

    void Texture::load_from_file(const std::string &file_path)
    {
        if(m_texture_id != 0)
            return;
        
        stbi_set_flip_vertically_on_load(true);

        unsigned char *data = stbi_load(m_file_path.c_str(), &m_width, &m_height, &m_bitDepth, STBI_rgb_alpha);
        if (!data)
        {
            prerr("Texture::load_from_file(): nothing here %s", m_file_path.c_str());
            return;
        }
        
        int internal_format = GL_RGBA;

        glGenTextures(1, &m_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0, internal_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }

    void Texture::use()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
    }

    void Texture::unuse()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::clear()
    {
        if (m_texture_id != 0)
            glDeleteTextures(1, &m_texture_id);
        m_texture_id = 0;
        m_height = 0;
        m_width = 0;
        m_bitDepth = 0;
        m_file_path.clear();
    }
    
    unsigned int Texture::id()
    {
        return m_texture_id;
    }
} // namespace nle
