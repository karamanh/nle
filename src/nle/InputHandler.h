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
    void set_key_state(int key, bool state);
    Signal<int> sig_key_pressed;
    Signal<int> sig_key_released;
    Signal<int, int> sig_mouse_moved;

    void set_mouse_state(int x, int y);

    // static void key_handler(GLFWwindow *window, int key, int code, int action, int mode);
    // static void mouse_handler(GLFWwindow *window, double mouse_x, double mouse_y);

public:
    InputHandler(GLFWwindow *handle);
    virtual ~InputHandler();

    const std::array<bool, 1024>& keys();
    bool key_state(int key);
    Signal<int>& key_pressed();
    Signal<int>& key_released();
    Signal<int,int>& mouse_moved();

    int mouse_x();
    int mouse_y();
    int mouse_dx();
    int mouse_dy();


    friend class Window;
};

} // namespace nle
