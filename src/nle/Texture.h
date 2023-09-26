#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

#include "Utils.h"

namespace nle
{
    class Texture
    {
    private:
        GLuint m_texture_id;
        GLint m_width;
        GLint m_height;
        GLint m_bitDepth;
        std::string m_file_path;

        void load_from_file(const std::string &file_path, bool flip);
        void load_from_memory(const unsigned char *blob, size_t size, bool flip);

    public:
        Texture(const std::string &file_path, bool flip = true);
        Texture(const unsigned char *blob, size_t size, bool flip = true);
        ~Texture();
        void use();
        void unuse();
        void clear();
        unsigned int id();

        friend class TextureFactory;
    };
} // namespace nle

#endif