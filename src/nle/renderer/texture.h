/**
 * @file texture.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <GL/glew.h>

#include <string>

namespace nle
{

class texture
{
public:
    texture(const std::string& path, bool flip = true);
    texture(const uint8_t *blob, size_t size, bool flip = true);

    /**
     * @brief Wraps pixels that are already decoded.
     *
     * glTF hands back decoded images rather than encoded files, so there is
     * nothing for stb to do. @p channels may be 1, 2, 3 or 4.
     */
    texture(const uint8_t *pixels, int width, int height, int channels, bool flip = false);

    virtual ~texture();

    uint32_t id() const;
    void use(uint8_t unit = 0) const;
    void unuse(uint8_t unit = 0) const;
private:
    uint32_t m_id = 0U;
    int m_width;
    int m_height;
    int m_bit_depth;

    void load_from_file(const std::string& path, bool flip);
    void load_from_memory(const unsigned char *blob, size_t size, bool flip);
    void load_from_pixels(const unsigned char *pixels, int width, int height, int channels, bool flip);

    /// Applies the wrap/filter parameters and uploads to the bound texture.
    void upload(const unsigned char *pixels, int format);
};

} // namespace nle
