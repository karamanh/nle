#include "texture.h"

#include "../core/utils.h"
#include "../../vendor/stb_image.h"

namespace nle
{
    texture::texture(const std::string &path, bool flip)
    {
        load_from_file(path, flip);
    }

    texture::texture(const uint8_t *blob, size_t size, bool flip)
    {
        load_from_memory(blob, size, flip);
    }

    texture::~texture()
    {
        glDeleteTextures(1, &m_id);
    }

    uint32_t texture::id() const
    {
        return m_id;
    }

    void texture::use(uint8_t unit) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    void texture::unuse(uint8_t unit) const
    {
        glBindTexture(GL_TEXTURE_2D + unit, 0);
    }

    void texture::load_from_file(const std::string &path, bool flip)
    {
        if(m_id != 0)
            return;
        
        stbi_set_flip_vertically_on_load(flip);

        unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &m_bit_depth, STBI_rgb_alpha);
        if (!data)
        {
            utils::prerror("Texture::load_from_file(): nothing here", path);
            return;
        }
        
        int internal_format = GL_RGBA;

        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0, internal_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }

    void texture::load_from_memory(const unsigned char *blob, size_t size, bool flip)
    {
        if(m_id != 0 || blob == nullptr || size == 0)
        {
            return;
        }

        stbi_set_flip_vertically_on_load(flip);

        unsigned char *data = stbi_load_from_memory(blob, size, &m_width, &m_height, &m_bit_depth, STBI_rgb_alpha);
        if(!data)
        {
            utils::prerror("Texture::load_from_memory(): error loading from memory");
            return;
        }
        
        int internal_format = GL_RGBA;

        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0, internal_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }

} // namespace nle
