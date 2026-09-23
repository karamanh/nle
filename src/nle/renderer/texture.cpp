#include "texture.h"

#include "../core/utils.h"
#include "../../vendor/stb_image.h"

#include <cstring>
#include <vector>

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

    texture::texture(const uint8_t *pixels, int width, int height, int channels, bool flip)
    {
        load_from_pixels(pixels, width, height, channels, flip);
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
        
        upload(data, GL_RGBA);

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
        
        upload(data, GL_RGBA);

        stbi_image_free(data);
    }

    void texture::load_from_pixels(const unsigned char *pixels, int width, int height, int channels, bool flip)
    {
        if(m_id != 0 || pixels == nullptr || width <= 0 || height <= 0)
        {
            return;
        }

        int format = GL_RGBA;
        switch(channels)
        {
            case 1: format = GL_RED; break;
            case 2: format = GL_RG; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default:
                utils::prerror("texture::load_from_pixels(): unsupported channel count", channels);
                return;
        }

        m_width = width;
        m_height = height;
        m_bit_depth = channels;

        std::vector<unsigned char> flipped;
        if(flip)
        {
            // stb's flip-on-load does not apply here; the rows are already decoded.
            const size_t stride = static_cast<size_t>(width) * static_cast<size_t>(channels);
            flipped.resize(stride * static_cast<size_t>(height));

            for(int y = 0; y < height; ++y)
            {
                std::memcpy(flipped.data() + stride * static_cast<size_t>(y),
                            pixels + stride * static_cast<size_t>(height - 1 - y),
                            stride);
            }

            pixels = flipped.data();
        }

        // rows of 1/2/3-channel images are not 4-byte aligned.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        upload(pixels, format);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    int texture::width() const
    {
        return m_width;
    }

    int texture::height() const
    {
        return m_height;
    }

    void texture::upload(const unsigned char *pixels, int format)
    {
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Mipmaps were always generated below and never used, which is why
        // ground seen at a distance boiled: a texel a pixel wide picked at
        // random every frame the camera moved. Magnification stays linear,
        // which is what the interface is drawn at.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

} // namespace nle
