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
#include "nle/mesh/mesh_3d.h"
#include "nle/renderer/texture.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <limits>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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

void test_nonsense_coordinates()
{
    std::cout << "\nimpossible positions\n";

    auto ground = make_ground();
    ground->sculpt({ 0.0f, 0.0f, 0.0f }, 20.0f, 6.0f, nle::sculpt_mode::raise, 1.0f);
    ground->set_paint_layers({ { "road", { 0.4f, 0.3f, 0.2f } } });
    ground->paint({ 0.0f, 0.0f, 0.0f }, 20.0f, 0, 0.5f, 1.0f);

    // A screen ray built from a cursor that has left the window is made of
    // infinities, and everything downstream inherits them. Sampling a
    // heightfield at one used to index it at INT_MIN and take the process
    // with it, because std::clamp passes a NaN straight through -- every
    // comparison against one being false -- and casting that to an int is
    // undefined.
    const float nan = std::nanf("");
    const float infinity = std::numeric_limits<float>::infinity();

    for(float bad : { nan, infinity, -infinity })
    {
        check(std::isfinite(ground->height_at(bad, 0.0f)) || ground->height_at(bad, 0.0f) == 0.0f,
              "height_at survives an impossible x");
        check(std::isfinite(ground->height_at(0.0f, bad)) || ground->height_at(0.0f, bad) == 0.0f,
              "height_at survives an impossible z");

        ground->place_on_surface({ bad, bad, bad });
        ground->slope_at(bad, bad);

        check(ground->paint_at(bad, bad) == -1, "paint_at reports nothing for one");
    }

    // Far outside the patch is not impossible, merely elsewhere, and has to
    // answer rather than reach past the end of the samples.
    check(std::isfinite(ground->height_at(1e9f, -1e9f)), "and a position a long way off");

    // A ray that cannot hit anything must say so rather than march forever.
    nle::ray nowhere;
    nowhere.origin = { nan, nan, nan };
    nowhere.direction = { nan, nan, nan };

    glm::vec3 hit(0.0f);
    check(!ground->raycast(nowhere, hit), "a ray made of nothing hits nothing");
}

/// A one-texel picture, so a layer can have one without a file to load.
nle::ref<nle::texture> a_picture(uint8_t red, uint8_t green, uint8_t blue)
{
    const uint8_t pixel[4] = { red, green, blue, 255 };
    return nle::make_ref<nle::texture>(pixel, 1, 1, 4, false);
}

/// The colour baked into the mesh nearest a spot, which is what the ground
/// is tinted with before any picture is sampled over it.
glm::vec3 colour_under(const nle::ref<nle::terrain_3d>& ground, float x, float z)
{
    const auto& vertices = ground->mesh()->vertices();

    glm::vec3 colour(0.0f);
    float nearest = std::numeric_limits<float>::max();

    for(const auto& one : vertices)
    {
        const float dx = one.position.x - x;
        const float dz = one.position.z - z;
        const float gap = dx * dx + dz * dz;

        if(gap < nearest)
        {
            nearest = gap;
            colour = one.color;
        }
    }

    return colour;
}

/// Every texel of a splat map, straight off the card.
std::vector<uint8_t> read_back(const nle::ref<nle::texture>& picture, int side)
{
    std::vector<uint8_t> texels(static_cast<size_t>(side) * static_cast<size_t>(side) * 4u, 0u);

    glBindTexture(GL_TEXTURE_2D, picture->id());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    return texels;
}

void test_the_splatmap()
{
    std::cout << "\nthe painting as the shader reads it\n";

    auto ground = make_ground();

    ground->set_paint_layers({ { "grass", { 0.2f, 0.5f, 0.2f } },
                               { "road", { 0.45f, 0.38f, 0.28f } } });
    ground->paint({ 0.0f, 0.0f, 0.0f }, 12.0f, 1, 0.4f, 1.0f);

    check(!ground->splatmap(), "no picture on any layer, so nothing would read a splat map");

    ground->set_layer_texture(1, a_picture(200, 120, 60));

    auto splat = ground->splatmap();
    check(static_cast<bool>(splat), "a layer with a picture is a reason to build one");

    if(!splat)
    {
        return;
    }

    const int n = ground->samples();

    // The whole point: channel g of texel (x, z) is layer one's weight at
    // sample (x, z). Were these ever to disagree, the ground would be painted
    // in one place and textured in another.
    std::vector<uint8_t> texels = read_back(splat, n);
    const auto& weights = ground->paintmap();

    bool same = true;
    bool anything = false;

    for(size_t sample = 0; sample < static_cast<size_t>(n) * static_cast<size_t>(n); ++sample)
    {
        for(size_t layer = 0; layer < 2u; ++layer)
        {
            const uint8_t painted = weights[sample * 2u + layer];

            same = same && texels[sample * 4u + layer] == painted;
            anything = anything || painted != 0u;
        }
    }

    check(anything, "the stroke left weights worth checking");
    check(same, "and every texel is the weight painted at that sample");

    // A texture is not a snapshot: paint again and the next read has it.
    ground->paint({ 20.0f, 0.0f, 20.0f }, 10.0f, 0, 0.6f, 1.0f);

    texels = read_back(ground->splatmap(), n);

    const auto& now = ground->paintmap();

    bool caught_up = true;
    bool moved = false;

    for(size_t sample = 0; sample < static_cast<size_t>(n) * static_cast<size_t>(n); ++sample)
    {
        caught_up = caught_up && texels[sample * 4u] == now[sample * 2u];
        moved = moved || now[sample * 2u] != 0u;
    }

    check(moved, "the second stroke painted the other layer");
    check(caught_up, "and the splat map followed it onto its own channel");
}

void test_pictures_are_not_tinted()
{
    std::cout << "\nwhat the mesh leaves for the pictures\n";

    auto plain = make_ground();
    plain->set_paint_layers({ { "grass", { 0.2f, 0.5f, 0.2f } },
                              { "road", { 0.45f, 0.38f, 0.28f } } });
    plain->paint({ 0.0f, 0.0f, 0.0f }, 12.0f, 1, 0.4f, 1.0f);

    auto pictured = make_ground();
    pictured->set_paint_layers({ { "grass", { 0.2f, 0.5f, 0.2f } },
                                 { "road", { 0.45f, 0.38f, 0.28f } } });
    pictured->paint({ 0.0f, 0.0f, 0.0f }, 12.0f, 1, 0.4f, 1.0f);
    pictured->set_layer_texture(1, a_picture(200, 120, 60));

    const glm::vec3 tinted = colour_under(plain, 0.0f, 0.0f);
    const glm::vec3 left_alone = colour_under(pictured, 0.0f, 0.0f);

    check(glm::length(tinted - left_alone) > 0.01f,
          "a layer with a picture stops tinting the mesh under it");

    // A base picture is a colour of ground, so the checkerboard under it has
    // to go, or it shows through everything painted on top.
    auto based = make_ground();
    based->set_base_texture(a_picture(90, 140, 70), 12.0f);

    check(glm::length(colour_under(based, 0.0f, 0.0f) - glm::vec3(1.0f)) < 0.01f,
          "and a base picture leaves the ground under it white");
}

void test_the_map_of_it()
{
    std::cout << "\nthe ground seen from above\n";

    auto ground = make_ground();

    ground->set_paint_layers({ { "grass", { 0.15f, 0.55f, 0.15f } },
                               { "road", { 0.60f, 0.35f, 0.10f } } });

    // A road across the middle, painted hard enough to cover what is under it.
    ground->paint({ 0.0f, 0.0f, 0.0f }, 14.0f, 1, 1.0f, 1.0f);

    const int side = 64;
    const auto picture = ground->overhead_image(side);

    check(picture.size() == static_cast<size_t>(side) * side * 4u,
          "the map is the size it was asked for");

    auto pixel = [&](int x, int y) {
        const size_t at = (static_cast<size_t>(y) * side + x) * 4u;
        return glm::vec3(picture[at] / 255.0f, picture[at + 1] / 255.0f,
                         picture[at + 2] / 255.0f);
    };

    const glm::vec3 middle = pixel(side / 2, side / 2);
    const glm::vec3 corner = pixel(2, 2);

    // The road is redder than what it was painted over, and the corner was
    // never painted at all. If these ever match, the map is not showing paint.
    check(middle.r > corner.r + 0.05f, "the road shows on the map");
    check(middle.r > middle.b, "and reads as the colour it was painted");

    check(picture[3] == 255, "every pixel is opaque");

    // The point of it: a layer drawn as a picture is still a colour on the
    // map. Painting is unchanged, only how it is drawn, so the map must not
    // change when a picture is put on the layer.
    const uint8_t pixel_before = picture[(static_cast<size_t>(side / 2) * side + side / 2) * 4u];

    const uint8_t one[4] = { 200, 120, 60, 255 };
    ground->set_layer_texture(1, nle::make_ref<nle::texture>(one, 1, 1, 4, false));

    const auto after = ground->overhead_image(side);
    const uint8_t pixel_after = after[(static_cast<size_t>(side / 2) * side + side / 2) * 4u];

    check(pixel_before == pixel_after,
          "and a layer given a picture still reads as its colour on the map");
}
} // namespace

void test_the_second_splatmap()
{
    std::cout << "\nthe layers past the fourth\n";

    auto ground = make_ground();

    // Six, which is two more than one splat map can carry.
    std::vector<nle::terrain_paint_layer> layers;

    for(int i = 0; i < 6; ++i)
    {
        nle::terrain_paint_layer one;
        one.name = "layer " + std::to_string(i);
        one.color = { 0.1f * i, 0.5f, 0.2f };
        layers.push_back(one);
    }

    ground->set_paint_layers(layers);

    // Painted on the fifth, which lives in the second map's first channel.
    ground->paint({ 0.0f, 0.0f, 0.0f }, 12.0f, 4, 1.0f, 1.0f);

    check(!ground->splatmap(1), "no picture on any of them, so no second map");

    const uint8_t pixel[4] = { 200, 120, 60, 255 };
    ground->set_layer_texture(4, nle::make_ref<nle::texture>(pixel, 1, 1, 4, false));

    check(static_cast<bool>(ground->splatmap(1)),
          "a picture on the fifth builds the second map");

    check(!ground->splatmap(0),
          "and the first stays unbuilt while nothing in it has one");

    const int n = ground->samples();
    const auto texels = read_back(ground->splatmap(1), n);

    const auto& weights = ground->paintmap();

    // The fifth layer is the second map's red channel: layer four of six,
    // which is channel nought of the map that starts at four.
    bool same = true;
    bool anything = false;

    for(size_t sample = 0; sample < static_cast<size_t>(n) * static_cast<size_t>(n); ++sample)
    {
        const uint8_t painted = weights[sample * 6u + 4u];

        same = same && texels[sample * 4u] == painted;
        anything = anything || painted != 0u;
    }

    check(anything, "the stroke left weights on the fifth layer");
    check(same, "and each is in the second map's first channel");

    // Nothing was painted on the sixth, so its channel is empty -- if the
    // packing were off by one, the fifth's weights would be sitting here.
    bool sixth_is_clear = true;

    for(size_t sample = 0; sample < static_cast<size_t>(n) * static_cast<size_t>(n); ++sample)
    {
        sixth_is_clear = sixth_is_clear && texels[sample * 4u + 1u] == 0u;
    }

    check(sixth_is_clear, "and the channel after it is untouched");
}

void test_the_edge_of_the_world()
{
    std::cout << "\nthe edge of the world\n";

    auto ground = make_ground();

    check_near(ground->bounds_radius(), 0.0f, "no edge to begin with");

    // With none, anywhere is somewhere, including well outside the mesh.
    const glm::vec3 far_away(9000.0f, 3.0f, -9000.0f);

    check(ground->inside_bounds(far_away) == far_away, "and nothing is brought back");

    ground->set_bounds_radius(30.0f);

    check_near(ground->bounds_radius(), 30.0f, "an edge can be set");

    // Inside is left exactly alone, height and all.
    const glm::vec3 within(10.0f, 4.5f, -10.0f);

    check(ground->inside_bounds(within) == within, "somewhere inside is left where it is");

    // Outside is brought to the edge, in the direction it went.
    const glm::vec3 out(100.0f, 7.0f, 0.0f);
    const glm::vec3 held = ground->inside_bounds(out);

    check_near(glm::length(glm::vec2(held.x, held.z)), 30.0f, "outside is brought to the edge");
    check_near(held.z, 0.0f, "along the way it was heading");
    check_near(held.y, 7.0f, "and its height is not the edge's business");

    // The corners are the point of a circle: every direction ends the same
    // distance out, which a square edge cannot do.
    const glm::vec3 corner = ground->inside_bounds({ 500.0f, 0.0f, 500.0f });

    check_near(glm::length(glm::vec2(corner.x, corner.z)), 30.0f,
               "and a corner is no further than a side");

    // Dead centre has no direction to be held in, and must not divide by it.
    const glm::vec3 middle = ground->inside_bounds({ 0.0f, 1.0f, 0.0f });

    check(std::isfinite(middle.x) && std::isfinite(middle.z),
          "the very middle is not a division by nothing");
}

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
    test_the_splatmap();
    test_the_map_of_it();
    test_the_second_splatmap();
    test_the_edge_of_the_world();
    test_pictures_are_not_tinted();
    test_nonsense_coordinates();

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";

    glfwDestroyWindow(window);
    glfwTerminate();

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
