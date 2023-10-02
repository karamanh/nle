#pragma once

#include "Signal.hpp"

#include <GLFW/glfw3.h>

#include <array>

namespace nle
{

class InputHandler
{
private:
    GLFWwindow *m_handle;

    int m_mouse_last_x;
    int m_mouse_last_y;
    int m_mouse_delta_x;
    int m_mouse_delta_y;
    bool m_mouse_moved_once = false;
    bool m_mouse_cam_lock = false;
    double m_input_timestamp;
    std::thread m_thr_input;
    std::array<bool, 1024> m_keys;
    std::array<bool, 8> m_mouse_buttons;
    Signal<int> sig_key_pressed;
    Signal<int> sig_key_just_pressed;
    Signal<int> sig_key_released;
    Signal<int, int> sig_mouse_moved;
    Signal<int, bool, int, int> sig_mouse_state_changed;

    void set_key_state(int key, bool state);
    void set_mouse_position(int x, int y);
    void set_mouse_button_state(int button, bool state);

public:
    InputHandler(GLFWwindow *handle);
    virtual ~InputHandler();

    const std::array<bool, 1024>& keys();
    bool key_state(int key);
    Signal<int>& key_pressed();
    Signal<int>& key_just_pressed();
    Signal<int>& key_released();
    Signal<int,int>& mouse_moved();
    Signal<int, bool, int, int>& mouse_button_state_changed();

    int mouse_x();
    int mouse_y();
    int mouse_dx();
    int mouse_dy();


    friend class Window;
};

} // namespace nle
