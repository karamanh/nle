/**
 * @file audio_test.cpp
 * @brief That a device opens, a track loads, and a handover happens.
 *
 * It really does play, briefly and quietly. There is no way to assert that a
 * speaker moved, but there is every way to assert that the mixer was given
 * something it accepted -- which is where this has gone wrong before.
 *
 * On a machine with no sound card the whole thing must still pass: ready()
 * says no, every call is a no-op, and nothing crashes. That is the case that
 * a build server actually runs.
 */

#include "nle/audio/audio.h"

#include <iostream>
#include <string>

namespace
{

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& what)
{
    ++g_checks;
    std::cout << (condition ? "  ok   " : "  FAIL ") << what << "\n";
    if(!condition)
    {
        ++g_failures;
    }
}

/// Winds the clock forward, as the game's frame would.
void run_for(nle::audio& sound, float seconds)
{
    const float dt = 1.0f / 60.0f;

    for(float t = 0.0f; t < seconds; t += dt)
    {
        sound.update(dt);
    }
}

} // namespace

int main(int argc, char** argv)
{
    const std::string track = argc > 1 ? argv[1] : std::string();

    nle::audio sound;

    if(!sound.ready())
    {
        std::cout << "  --   no audio device here; checking it stays quiet\n";

        sound.play_music("anything.mp3");
        sound.update(1.0f / 60.0f);

        check(!sound.music_playing(), "silence, and no crash, without a device");

        std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
        return g_failures == 0 ? 0 : 1;
    }

    check(sound.ready(), "a device opened");

    if(track.empty())
    {
        std::cout << "  --   no track given; skipping the playing checks\n";
        std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
        return g_failures == 0 ? 0 : 1;
    }

    // Quietly. This runs on somebody's desk.
    sound.set_music_volume(0.02f);

    sound.play_music(track, 0.25f);

    check(sound.music() == track, "a track can be asked for");
    check(!sound.music_playing(), "and nothing is sounding until the clock turns");

    run_for(sound, 0.5f);

    check(sound.music_playing(), "it is playing once it has been given a moment");

    // Asking again for what is already on must not restart it.
    sound.play_music(track, 0.25f);
    run_for(sound, 0.1f);

    check(sound.music_playing(), "asking for the same track again changes nothing");

    sound.stop_music(0.25f);

    check(sound.music().empty(), "stopping forgets what was wanted");

    run_for(sound, 0.6f);

    // Halted at the bottom of the fade rather than left running silently:
    // a track nobody can hear is still a track being decoded.
    check(!sound.music_playing(), "and the fade actually ends in silence");

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
