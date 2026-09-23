#include "splash.h"

#include "core/utils.h"

#include <algorithm>

namespace nle
{

namespace
{
    /// How much of its life is spent arriving and leaving. The middle is held
    /// at full strength: a card that spends all its time fading is a card
    /// nobody managed to read.
    constexpr float IN = 0.18f;
    constexpr float OUT = 0.30f;
}

splash::splash(ref<class renderer_2d> renderer, const std::string& picture, float seconds)
    : m_renderer(std::move(renderer)), m_seconds(std::max(0.0f, seconds))
{
    if(m_seconds <= 0.0f || !m_renderer)
    {
        m_seconds = 0.0f;
        return;
    }

    try
    {
        m_picture = make_ref<class texture>(picture, false);
        m_width = m_picture->width();
        m_height = m_picture->height();
    }
    catch(const std::exception& e)
    {
        utils::prerror("splash: could not load", picture, ":", e.what());
        m_picture = nullptr;
        m_seconds = 0.0f;
    }
}

bool splash::finished() const
{
    return m_elapsed >= m_seconds || !m_picture;
}

bool splash::draw(const glm::vec2& resolution, float delta_time)
{
    if(finished())
    {
        return false;
    }

    m_elapsed += delta_time;

    const float t = std::clamp(m_elapsed / m_seconds, 0.0f, 1.0f);

    float showing = 1.0f;

    if(t < IN)
    {
        showing = t / IN;
    }
    else if(t > 1.0f - OUT)
    {
        showing = (1.0f - t) / OUT;
    }

    showing = std::clamp(showing, 0.0f, 1.0f);

    m_renderer->begin(resolution);

    // Black under it, at the same strength, so what is behind is covered
    // while the card is up and revealed as it goes rather than all at once.
    m_renderer->draw_rect({ 0.0f, 0.0f }, resolution, { 0.0f, 0.0f, 0.0f, showing });

    if(m_width > 0 && m_height > 0)
    {
        // Fitted inside the window with its shape kept. A logo stretched to
        // the window is a logo drawn wrong, and every window is a different
        // shape from every other.
        const float scale = std::min(resolution.x / static_cast<float>(m_width),
                                     resolution.y / static_cast<float>(m_height));

        const glm::vec2 size(static_cast<float>(m_width) * scale,
                             static_cast<float>(m_height) * scale);

        m_renderer->draw_texture((resolution - size) * 0.5f, size, m_picture,
                                 { 1.0f, 1.0f, 1.0f, showing });
    }

    m_renderer->end();

    return true;
}

} // namespace nle
