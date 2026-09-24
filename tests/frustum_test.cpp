/**
 * @file frustum_test.cpp
 * @brief Checks what a camera can see, and what it only paid to draw.
 *
 * No GL context: a frustum is six planes taken from two matrices, and the
 * question it answers is arithmetic. The camera here looks down -z from the
 * origin, which is where glm::lookAt with no turning puts it.
 */

#include "nle/renderer/frustum.h"

#include <glm/gtc/matrix_transform.hpp>

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

/// Sixty degrees over a square window, seeing from one unit out to a hundred.
nle::frustum looking_down_negative_z()
{
    const glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 100.0f);

    const glm::mat4 view = glm::lookAt(glm::vec3(0.0f),
                                       glm::vec3(0.0f, 0.0f, -1.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f));

    return nle::frustum::of(projection * view);
}

void test_what_is_in_front_of_you()
{
    std::cout << "\nwhat the camera can see\n";

    const nle::frustum view = looking_down_negative_z();

    check(view.holds({ 0.0f, 0.0f, -10.0f }, 1.0f),
          "something straight ahead is in view");

    // The whole point of the exercise. A render distance keeps this.
    check(!view.holds({ 0.0f, 0.0f, 10.0f }, 1.0f),
          "and the same thing behind you is not");

    check(!view.holds({ 40.0f, 0.0f, -10.0f }, 1.0f),
          "nor is something off to the side, however close");

    check(!view.holds({ 0.0f, 0.0f, -400.0f }, 1.0f),
          "nor something past the far plane");
}

void test_a_radius_counts()
{
    std::cout << "\nhow big it is counts\n";

    const nle::frustum view = looking_down_negative_z();

    // Half on screen is on screen. Testing the centre alone would cut the
    // corner off a building as soon as its middle left the window.
    check(view.holds({ 0.0f, 0.0f, 4.0f }, 12.0f),
          "something behind you that is big enough to reach in front is in view");

    check(!view.holds({ 0.0f, 0.0f, 4.0f }, 1.0f),
          "and the same place with a small one is not");

    // The side plane of a sixty degree view at ten units out stands about
    // 5.77 from the axis. A sphere whose radius reaches it is in; one just
    // short of it is out. This is what says the planes are measured in world
    // units rather than in whatever scale the matrix arrived carrying -- an
    // unnormalised plane still sorts in from out, but not at the right place.
    const float edge = 10.0f * std::tan(glm::radians(30.0f));

    check(view.holds({ edge + 2.0f, 0.0f, -10.0f }, 2.2f),
          "a sphere reaching the edge of the view is in it");

    check(!view.holds({ edge + 2.0f, 0.0f, -10.0f }, 1.5f),
          "and one falling short of it is not");
}

} // namespace

int main()
{
    test_what_is_in_front_of_you();
    test_a_radius_counts();

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
