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
    texture(const std::string& path, bool flip);
    texture(const uint8_t *blob, size_t size, bool flip);
    virtual ~texture();

    uint32_t id() const;
    void use(uint8_t unit = 0);
    void unuse(uint8_t unit = 0);
private:
    uint32_t m_id;
    int m_width;
    int m_height;
    int m_bit_depth;

    void load_from_file(const std::string& path, bool flip);
    void load_from_memory(const unsigned char *blob, size_t size, bool flip);
};

} // namespace nle
