/**
 * @file terrain_sculpt_test.cpp
 * @brief Checks the editable heightfield and the brush that pushes it around.
 *
 * Building a terrain uploads a mesh, so this opens a hidden window rather than
 * running purely on the CPU, and reports 77 when there is no display.
 *
 * The property that matters throughout is that what is drawn and what is
 * queried come from the same samples: a brush that raises ground you then
 * fall through would be worse than no brush at all.
 */

#include "nle/scene/terrain_3d.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
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

void check_near(float actual, float expected, const std::string& what, float tolerance = 1e-3f)
{
    const bool ok = std::fabs(actual - expected) <= tolerance;
    ++g_checks;
    std::cout << (ok ? "  ok   " : "  FAIL ") << what;
    if(!ok)
    {
        std::cout << " (expected " << expected << ", got " << actual << ")";
    }
    std::cout << "\n";
    if(!ok)
    {
        ++g_failures;
    }
}

nle::ref<nle::terrain_3d> make_ground()
{
    // 100 units over 50 tiles: a sample every two units, which is fine enough
    // to see a brush fall off and coarse enough to reason about by hand.
    auto ground = nle::make_ref<nle::terrain_3d>(100.0f, 50);
    ground->set_layers({ { 0.0f, { 0.3f, 0.4f, 0.2f } }, { 10.0f, { 0.6f, 0.6f, 0.6f } } });
    return ground;
}

void test_heightmap_basics()
{
    std::cout << "\nthe heightfield\n";

    auto ground = make_ground();

    check(!ground->sculpted(), "flat ground has no heightfield yet");
    check(ground->samples() == 51, "a 50 tile edge has 51 samples");
    check(ground->heightmap().empty(), "and the field is empty until asked for");

    ground->bake_heightmap();

    check(ground->sculpted(), "baking gives it one");
    check(ground->heightmap().size() == 51u * 51u, "of one height per sample");

    // Baking flat ground must produce flat ground, not an approximation of it.
    bool all_flat = true;
    for(float h : ground->heightmap())
    {
        if(std::fabs(h) > 1e-6f) { all_flat = false; }
    }
    check(all_flat, "baking flat ground gives flat samples");

    check(!ground->set_heightmap(std::vector<float>(10, 0.0f)),
          "a wrong-sized heightfield is refused");
    check(ground->set_heightmap(std::vector<float>(51u * 51u, 3.0f)),
          "a right-sized one is taken");
    check_near(ground->height_at(0.0f, 0.0f), 3.0f, "and is what height_at answers from");
    check_near(ground->height_at(17.3f, -8.1f), 3.0f, "anywhere on it");
}

void test_noise_is_the_starting_point()
{
    std::cout << "\nfrom noise to samples\n";

    auto ground = make_ground();
    ground->set_noise({ .seed = 7, .frequency = 0.03f, .amplitude = 8.0f });

    // What the generated surface says, before anything freezes it.
    const float generated = ground->height_at(12.0f, -6.0f);

    ground->bake_heightmap();

    check(ground->sculpted(), "baking noise gives a field to sculpt");
    check_near(ground->height_at(12.0f, -6.0f), generated,
               "and the ground does not move when it is frozen", 0.05f);

    // Regenerating is starting again, so sculpting cannot survive it.
    ground->sculpt({ 0.0f, 0.0f, 0.0f }, 10.0f, 50.0f, nle::sculpt_mode::raise, 1.0f);
    check(ground->sculpted(), "sculpting keeps the field");

    ground->set_noise({ .seed = 9, .frequency = 0.03f, .amplitude = 8.0f });
    check(!ground->sculpted(), "regenerating throws it away");

    ground->bake_heightmap();
    ground->clear_heightmap();
    check(!ground->sculpted(), "and so does clearing it");
}

void test_raise_and_lower()
{
    std::cout << "\nraising and lowering\n";

    auto ground = make_ground();

    const glm::vec3 centre(0.0f, 0.0f, 0.0f);
    const float radius = 10.0f;

    check(ground->sculpt(centre, radius, 4.0f, nle::sculpt_mode::raise, 1.0f),
          "a stroke reports that it moved something");

    const float peak = ground->height_at(0.0f, 0.0f);
    const float middle = ground->height_at(5.0f, 0.0f);
    const float rim = ground->height_at(9.5f, 0.0f);

    check(peak > 3.9f && peak < 4.1f, "the centre rises by the full strength");
    check(middle > 0.0f && middle < peak, "halfway out rises less");
    check(rim >= 0.0f && rim < middle, "the rim barely moves");

    // Outside the brush nothing may move at all, or strokes would drag the
    // whole map about.
    check_near(ground->height_at(radius + 1.0f, 0.0f), 0.0f, "outside the brush is untouched");
    check_near(ground->height_at(0.0f, 40.0f), 0.0f, "and so is the far side");

    // Strokes accumulate, which is what makes a hill out of a brush.
    ground->sculpt(centre, radius, 4.0f, nle::sculpt_mode::raise, 1.0f);
    check(ground->height_at(0.0f, 0.0f) > peak, "a second stroke builds on the first");

    const float before_lowering = ground->height_at(0.0f, 0.0f);
    ground->sculpt(centre, radius, 4.0f, nle::sculpt_mode::lower, 1.0f);
    check(ground->height_at(0.0f, 0.0f) < before_lowering, "lowering undoes it");

    // Time is what strength is measured against, so half the time is half
    // the effect -- which is what keeps a brush frame-rate independent.
    auto fresh = make_ground();
    fresh->sculpt(centre, radius, 4.0f, nle::sculpt_mode::raise, 0.5f);
    check_near(fresh->height_at(0.0f, 0.0f), 2.0f, "half the time is half the height");
}

void test_flatten_and_smooth()
{
    std::cout << "\nflattening and smoothing\n";

    auto ground = make_ground();

    ground->sculpt({ 0.0f, 0.0f, 0.0f }, 12.0f, 9.0f, nle::sculpt_mode::raise, 1.0f);
    check(ground->height_at(0.0f, 0.0f) > 8.0f, "a mound to work on");

    // Flatten pulls towards the height it is given, and must approach it
    // rather than overshoot however long it is held.
    for(int i = 0; i < 200; ++i)
    {
        ground->sculpt({ 0.0f, 0.0f, 0.0f }, 12.0f, 5.0f, nle::sculpt_mode::flatten, 1.0f / 60.0f,
                       2.0f);
    }

    check_near(ground->height_at(0.0f, 0.0f), 2.0f, "flatten settles at the height asked for", 0.1f);
    check(ground->height_at(0.0f, 0.0f) <= 2.0001f, "and does not overshoot it");

    // Smoothing must reduce how much the surface varies, which is the only
    // thing it really promises.
    auto rough = make_ground();
    rough->bake_heightmap();

    auto spikes = rough->heightmap();
    for(size_t i = 0; i < spikes.size(); ++i)
    {
        spikes[i] = (i % 2 == 0) ? 5.0f : -5.0f;
    }
    rough->set_heightmap(spikes);

    auto roughness_of = [&](const nle::ref<nle::terrain_3d>& t) {
        float total = 0.0f;
        const auto& h = t->heightmap();
        for(size_t i = 1; i < h.size(); ++i)
        {
            total += std::fabs(h[i] - h[i - 1]);
        }
        return total;
    };

    const float before = roughness_of(rough);

    for(int i = 0; i < 40; ++i)
    {
        rough->sculpt({ 0.0f, 0.0f, 0.0f }, 60.0f, 10.0f, nle::sculpt_mode::smooth, 1.0f / 30.0f);
    }

    check(roughness_of(rough) < before * 0.5f, "smoothing takes the spikes out");
}

void test_what_is_drawn_is_what_is_walked_on()
{
    std::cout << "\nagreement\n";

    auto ground = make_ground();
    ground->sculpt({ 6.0f, 0.0f, -4.0f }, 14.0f, 7.0f, nle::sculpt_mode::raise, 1.0f);

    // place_on_surface, height_at and the normal all have to come from the
    // same samples, or things sit above or inside the hill they are put on.
    const glm::vec3 placed = ground->place_on_surface({ 6.0f, 0.0f, -4.0f });
    check_near(placed.y, ground->height_at(6.0f, -4.0f), "place_on_surface agrees with height_at");

    // A ray straight down must land on the surface, not on where it used to be.
    nle::ray down;
    down.origin = { 6.0f, 200.0f, -4.0f };
    down.direction = { 0.0f, -1.0f, 0.0f };

    glm::vec3 hit(0.0f);
    check(ground->raycast(down, hit), "a ray finds the sculpted ground");
    check_near(hit.y, ground->height_at(6.0f, -4.0f), "where the surface actually is", 0.05f);

    // The slope of a hillside must not still read as flat.
    check(ground->slope_at(13.0f, -4.0f) > 5.0f, "the side of the mound is sloped");
    check(ground->slope_at(60.0f, 60.0f) < 1.0f, "and untouched ground is not");
}

void test_painting()
{
    std::cout << "\npainting\n";

    auto ground = make_ground();

    check(!ground->painted(), "nothing is painted to begin with");
    check(!ground->paint({ 0.0f, 0.0f, 0.0f }, 10.0f, 0, 0.4f, 1.0f),
          "and painting does nothing without a layer to paint with");

    ground->set_paint_layers({ { "grass", { 0.2f, 0.5f, 0.2f } },
                               { "road", { 0.45f, 0.38f, 0.28f } } });

    check(ground->paint_layers().size() == 2, "two layers to paint with");
    check(ground->paint_at(0.0f, 0.0f) == -1, "and still nothing painted anywhere");

    // Gentle enough not to saturate: a stroke that pins every sample to full
    // strength has no falloff left to see.
    check(ground->paint({ 0.0f, 0.0f, 0.0f }, 12.0f, 1, 0.4f, 1.0f), "a stroke paints");
    check(ground->painted(), "which is now something");
    check(ground->paint_at(0.0f, 0.0f) == 1, "the road is what is underfoot at the centre");
    check(ground->paint_at(40.0f, 40.0f) == -1, "and nothing is, well outside the brush");

    // The falloff has to leave the rim lighter than the middle, or strokes
    // stack into visible discs instead of blending.
    const auto& weights = ground->paintmap();
    const int n = ground->samples();
    const int middle = n / 2;

    auto weight_at = [&](int x, int z, int layer) {
        return weights[(static_cast<size_t>(z) * static_cast<size_t>(n)
                      + static_cast<size_t>(x)) * 2u + static_cast<size_t>(layer)];
    };

    check(weight_at(middle, middle, 1) > weight_at(middle + 4, middle, 1),
          "the brush falls off towards its rim");
    check(weight_at(middle, middle, 0) == 0, "and leaves the layer it was not painting alone");

    // Erasing takes it off again rather than revealing something else.
    const uint8_t before = weight_at(middle, middle, 1);
    ground->paint({ 0.0f, 0.0f, 0.0f }, 12.0f, 1, 0.15f, 0.5f, true);

    const uint8_t after_erasing = weight_at(middle, middle, 1);
    check(after_erasing < before, "erasing takes it off again");
    check(after_erasing > 0, "a light touch thins it rather than stripping it");

    // Changing the set of layers must not wipe what is already painted.
    ground->set_paint_layers({ { "grass", { 0.2f, 0.5f, 0.2f } },
                               { "road", { 0.5f, 0.4f, 0.3f } },
                               { "sand", { 0.8f, 0.75f, 0.5f } } });

    check(ground->paint_at(0.0f, 0.0f) == 1, "adding a layer keeps the painting");
    check(ground->paintmap().size() == static_cast<size_t>(n) * n * 3u,
          "and the weights grow to fit");

    check(!ground->set_paintmap(std::vector<uint8_t>(10, 0)),
          "a wrong-sized paintmap is refused");
    check(ground->set_paintmap(std::vector<uint8_t>(static_cast<size_t>(n) * n * 3u, 0)),
          "a right-sized one is taken");
    check(ground->paint_at(0.0f, 0.0f) == -1, "and replaces what was there");
}

} // namespace

int main()
{
    if(!glfwInit())
    {
        std::cerr << "no glfw\n";
        return 77;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(64, 64, "terrain sculpt test", nullptr, nullptr);

    if(!window)
    {
        std::cerr << "no window\n";
        glfwTerminate();
        return 77;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if(glewInit() != GLEW_OK)
    {
        std::cerr << "no glew\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 77;
    }

    test_heightmap_basics();
    test_noise_is_the_starting_point();
    test_raise_and_lower();
    test_flatten_and_smooth();
    test_what_is_drawn_is_what_is_walked_on();
    test_painting();

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";

    glfwDestroyWindow(window);
    glfwTerminate();

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
