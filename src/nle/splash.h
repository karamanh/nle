/**
 * @file splash.h
 * @brief The engine's own card, shown once while a game is starting up.
 */

#pragma once

#include "core/ref.h"
#include "renderer/renderer_2d.h"
#include "renderer/texture.h"

#include <string>

namespace nle
{

/**
 * @brief A picture over the top of everything, for a moment, then gone.
 *
 * Shown by nle at startup without being asked for, which is the point of it:
 * a game built on this engine says so without its author having to remember
 * to say it. Passing a different picture, or nought seconds, is how a game
 * that has its own opinion about that expresses it.
 *
 * It holds the screen rather than the loop: the game carries on loading
 * behind it, and what the player sees while that happens is this rather than
 * an empty window. That also means it cannot be used to hide anything -- it
 * fades out on a clock, not when loading finishes.
 */
class splash
{
public:
    /**
     * @param picture what to show. A missing file is not an error: the splash
     *                simply does not happen, because a game that will not
     *                start over a logo is worse than a game with no logo.
     */
    splash(ref<class renderer_2d> renderer, const std::string& picture, float seconds);

    /// Moves it along. False once there is nothing left to show.
    bool draw(const glm::vec2& resolution, float delta_time);

    bool finished() const;

private:
    ref<class renderer_2d> m_renderer;
    ref<class texture> m_picture;

    float m_elapsed = 0.0f;
    float m_seconds = 0.0f;

    int m_width = 0;
    int m_height = 0;
};

} // namespace nle
