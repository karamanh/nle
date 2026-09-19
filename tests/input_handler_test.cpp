/**
 * @file input_handler_test.cpp
 * @brief Checks that the input signals report edges rather than levels.
 *
 * Both the keyboard and the mouse are polled every frame, so a signal that
 * emitted whatever state it found would fire continuously for everything,
 * pressed or not. A handler written as "the button just went down" would then
 * run on every frame the button was up, which is a difficult sort of bug to
 * see: nothing crashes, the handler simply runs about sixty times a second
 * when it should not run at all.
 *
 * Headless: input_handler is a template over the window handle, so the test
 * instantiates it over an int and drives the protected setters itself.
 */

#include "nle/window/input_handler.hpp"

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

void check_equal(int actual, int expected, const std::string& what)
{
    const bool ok = actual == expected;
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

/// Exposes the protected setters a real handler would call from its poll.
class test_handler : public nle::input_handler<int>
{
public:
    test_handler() : nle::input_handler<int>(0) {}

    using nle::input_handler<int>::set_mouse_button_state;
    using nle::input_handler<int>::set_key_state;
    using nle::input_handler<int>::set_mouse_position;

    void poll_keyboard_input() override {}
    void poll_mouse_input() override {}
};

void test_mouse_buttons_report_edges()
{
    std::cout << "\nmouse buttons\n";

    test_handler input;

    int downs = 0;
    int ups = 0;

    input.sig_mouse_state_changed.bind_callback([&](int, bool down, double, double) {
        if(down) ++downs; else ++ups;
    });

    // The state every frame of a poll loop with nobody touching the mouse.
    for(int frame = 0; frame < 60; ++frame)
    {
        input.set_mouse_button_state(0, false);
    }
    check_equal(downs + ups, 0, "an untouched button never emits");

    // Pressed, then held for a second at sixty frames a second.
    input.set_mouse_button_state(0, true);
    for(int frame = 0; frame < 60; ++frame)
    {
        input.set_mouse_button_state(0, true);
    }
    check_equal(downs, 1, "a press emits once, however long it is held");
    check_equal(ups, 0, "and holding never reports a release");

    // Released, then left alone.
    input.set_mouse_button_state(0, false);
    for(int frame = 0; frame < 60; ++frame)
    {
        input.set_mouse_button_state(0, false);
    }
    check_equal(ups, 1, "a release emits once");
    check_equal(downs, 1, "and does not re-report the press");

    check(input.mouse_button_down(0) == false, "the polled state follows the last set");

    input.set_mouse_button_state(0, true);
    check(input.mouse_button_down(0) == true, "and follows it back");
}

void test_buttons_are_independent()
{
    std::cout << "\nbutton independence\n";

    test_handler input;

    std::vector<int> emissions(8, 0);

    input.sig_mouse_state_changed.bind_callback([&](int button, bool, double, double) {
        if(button >= 0 && button < 8)
        {
            ++emissions[static_cast<size_t>(button)];
        }
    });

    // One button goes down while a real poll reports all eight every frame.
    for(int frame = 0; frame < 30; ++frame)
    {
        for(int button = 0; button < 8; ++button)
        {
            input.set_mouse_button_state(button, button == 1);
        }
    }

    check_equal(emissions[1], 1, "the button that changed emits once");

    int others = 0;
    for(int button = 0; button < 8; ++button)
    {
        if(button != 1)
        {
            others += emissions[static_cast<size_t>(button)];
        }
    }
    check_equal(others, 0, "the seven nobody touched stay silent");
}

void test_keys_report_edges()
{
    std::cout << "\nkeys\n";

    test_handler input;

    int just_pressed = 0;
    int released = 0;

    input.sig_key_just_pressed.bind_callback([&](const int&) { ++just_pressed; });
    input.sig_key_released.bind_callback([&](const int&) { ++released; });

    input.set_key_state(42, true);
    for(int frame = 0; frame < 60; ++frame)
    {
        input.set_key_state(42, true);
    }
    check_equal(just_pressed, 1, "a held key is 'just pressed' once");

    input.set_key_state(42, false);
    for(int frame = 0; frame < 60; ++frame)
    {
        input.set_key_state(42, false);
    }
    check_equal(released, 1, "and released once");

    check(input.key_down(42) == false, "the polled state follows the last set");
}

} // namespace

int main()
{
    test_mouse_buttons_report_edges();
    test_buttons_are_independent();
    test_keys_report_edges();

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
