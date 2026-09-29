/**
 * @file render_texture.h
 * @brief Somewhere to draw a scene other than the window.
 *
 * A colour texture with a depth buffer behind it. What is drawn into it can
 * be shown anywhere a texture can -- a character standing in a menu, a
 * portrait in a panel -- while the window goes on showing its own scene.
 */

#pragma once

namespace nle
{

class render_texture
{
public:
    render_texture();
    ~render_texture();

    render_texture(const render_texture&) = delete;
    render_texture& operator=(const render_texture&) = delete;

    /**
     * @brief Makes it @p width by @p height, or leaves it be if it already is.
     *
     * Cheap to call every frame with the size it is to be shown at.
     *
     * @return whether there is somewhere to draw.
     */
    bool resize(int width, int height);

    unsigned int framebuffer() const;

    /// The colour, as a GL texture name, with see-through where nothing
    /// was drawn.
    unsigned int texture() const;

    int width() const;
    int height() const;

private:
    void destroy();

    unsigned int m_framebuffer = 0;
    unsigned int m_color = 0;
    unsigned int m_depth = 0;

    int m_width = 0;
    int m_height = 0;
};

} // namespace nle
