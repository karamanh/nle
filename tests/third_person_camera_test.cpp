/**
 * @file third_person_camera_test.cpp
 * @brief Checks the orbit limits and the spring arm.
 *
 * No GL context and no window: the camera is a transform and some arithmetic,
 * and the obstruction it reacts to comes in through a callback, so a stub
 * stands in for the world. Terrain collision is not covered here -- building a
 * terrain uploads a mesh, which does need a context.
 */

#include "nle/scene/camera/third_person_camera.h"

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

/// A camera with nothing in its way, parked at a known pose.
nle::ref<nle::third_person_camera> make_camera()
{
    auto view = nle::make_ref<nle::third_person_camera>();

    view->set_target({ 0.0f, 0.0f, 0.0f });
    view->set_pivot_height(2.0f);
    view->set_distance(10.0f);
    view->set_yaw(0.0f);
    view->set_pitch(0.0f);
    view->snap();

    return view;
}

void test_pitch_limits()
{
    std::cout << "\npitch limits\n";

    auto view = make_camera();

    // The default limits are what keep the camera off the poles, where the
    // view would roll over.
    check_near(view->minimum_pitch(), -80.0f, "defaults to a floor of -80");
    check_near(view->maximum_pitch(), 80.0f, "defaults to a ceiling of +80");

    view->set_pitch(1000.0f);
    check_near(view->pitch(), 80.0f, "pitching far up stops at the ceiling");

    view->set_pitch(-1000.0f);
    check_near(view->pitch(), -80.0f, "pitching far down stops at the floor");

    // Dragging is the way this actually gets hit, so drive it that way too.
    view->set_pitch(0.0f);
    for(int i = 0; i < 200; ++i)
    {
        view->orbit(0.0f, 50.0f);
    }
    check_near(view->pitch(), 80.0f, "dragging up cannot pass the ceiling");

    for(int i = 0; i < 400; ++i)
    {
        view->orbit(0.0f, -50.0f);
    }
    check_near(view->pitch(), -80.0f, "dragging down cannot pass the floor");

    // Tightening the limits must not leave the current pitch outside them.
    view->set_pitch(0.0f);
    view->set_pitch_limits(-20.0f, 10.0f);
    check_near(view->maximum_pitch(), 10.0f, "limits can be narrowed");

    view->set_pitch(60.0f);
    check_near(view->pitch(), 10.0f, "the narrowed ceiling applies");

    view->set_pitch(0.0f);
    view->set_pitch_limits(5.0f, 40.0f);
    check_near(view->pitch(), 5.0f, "raising the floor pulls the pitch up with it");

    // Given backwards, the limits still mean the same range.
    view->set_pitch_limits(70.0f, -70.0f);
    check_near(view->minimum_pitch(), -70.0f, "limits given backwards are sorted");
    check_near(view->maximum_pitch(), 70.0f, "limits given backwards are sorted");
}

void test_orbit()
{
    std::cout << "\norbit\n";

    auto view = make_camera();

    view->set_sensitivity(0.25f);
    view->set_yaw(0.0f);
    view->orbit(40.0f, 0.0f);
    check_near(view->yaw(), 10.0f, "yaw follows the mouse by the sensitivity");

    // Yaw has no limits, it wraps.
    view->set_yaw(350.0f);
    view->orbit(80.0f, 0.0f);
    check_near(view->yaw(), 10.0f, "yaw wraps past a full turn");

    view->set_yaw(-90.0f);
    check_near(view->yaw(), 270.0f, "negative yaw wraps into range");

    bool in_range = true;
    for(int i = 0; i < 500; ++i)
    {
        view->orbit(137.0f, 0.0f);

        if(view->yaw() < 0.0f || view->yaw() >= 360.0f)
        {
            in_range = false;
        }
    }
    check(in_range, "yaw stays in [0, 360) however far it is dragged");

    // Yaw zero puts the camera behind a model that faces +Z.
    view->set_yaw(0.0f);
    view->set_pitch(0.0f);
    view->snap();

    const glm::vec3 placed = view->position();
    check_near(placed.x, 0.0f, "at yaw zero the camera is on the Z axis");
    check_near(placed.y, 2.0f, "level with the pivot when the pitch is zero");
    check_near(placed.z, 10.0f, "a full arm length behind the pivot");

    // A quarter turn should swing it onto +X.
    view->set_yaw(90.0f);
    view->snap();
    check_near(view->position().x, 10.0f, "a quarter turn swings the arm onto +X");
    check_near(view->position().z, 0.0f, "and off the Z axis");

    // Whatever the angles, the camera looks back at what it orbits.
    view->set_yaw(37.0f);
    view->set_pitch(55.0f);
    view->snap();

    const glm::vec3 pivot = view->target() + glm::vec3(0.0f, view->pivot_height(), 0.0f);
    const glm::vec3 to_pivot = glm::normalize(pivot - view->position());

    check_near(glm::dot(to_pivot, view->front()), 1.0f, "the camera faces its pivot", 1e-3f);
    check_near(glm::distance(view->position(), pivot), view->arm_length(),
               "and sits exactly an arm away");
}

void test_spring_arm()
{
    std::cout << "\nspring arm\n";

    auto view = make_camera();
    view->set_return_speed(4.0f);

    float blocked_at = 0.0f;
    bool blocking = false;

    view->set_collision_probe([&](const nle::ray&, float, float& out) {
        out = blocked_at;
        return blocking;
    });

    check_near(view->arm_length(), 10.0f, "starts at its full length");

    // Something gets in the way: the arm must give at once, or the camera
    // spends the ease inside it.
    blocking = true;
    blocked_at = 4.0f;
    view->update(1.0f / 60.0f);
    check_near(view->arm_length(), 4.0f, "collapses to an obstruction immediately");

    const glm::vec3 pivot = view->target() + glm::vec3(0.0f, view->pivot_height(), 0.0f);
    check_near(glm::distance(view->position(), pivot), 4.0f, "and the camera comes with it");

    // Closer still, same frame behaviour.
    blocked_at = 2.0f;
    view->update(1.0f / 60.0f);
    check_near(view->arm_length(), 2.0f, "keeps up as the obstruction closes in");

    // Cleared: it eases back rather than snapping, at the return speed.
    blocking = false;
    view->update(0.5f);
    check_near(view->arm_length(), 4.0f, "eases back out at the return speed");

    view->update(0.5f);
    check_near(view->arm_length(), 6.0f, "and keeps easing");

    // It must stop at the distance asked for, not sail past it.
    for(int i = 0; i < 100; ++i)
    {
        view->update(0.5f);
    }
    check_near(view->arm_length(), 10.0f, "settles at the requested distance");

    // An obstruction closer than the minimum still leaves the camera out at
    // the minimum rather than collapsing onto the pivot.
    view->set_minimum_distance(3.0f);
    blocking = true;
    blocked_at = 0.1f;
    view->update(1.0f / 60.0f);
    check_near(view->arm_length(), 3.0f, "never collapses past the minimum distance");

    // The requested distance is held above the minimum too.
    view->set_minimum_distance(5.0f);
    view->set_distance(1.0f);
    check_near(view->distance(), 5.0f, "the distance cannot be set below the minimum");
}

} // namespace

int main()
{
    test_pitch_limits();
    test_orbit();
    test_spring_arm();

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
