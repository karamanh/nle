#include "InputHandler.h"

#define NLE_INPUT_PROCESS_SLEEP_TIME_MS     16666

namespace nle
{

InputHandler::InputHandler(GLFWwindow *handle)
    : m_handle(handle)
{
}

InputHandler::~InputHandler()
{
}

void InputHandler::set_key_state(int key, bool state)
{
    if(state)
    {
        if(m_keys[key] != state)
        {
            m_keys[key] = state;
            sig_key_just_pressed.emit(key, false);
        }
        else
        {
            sig_key_pressed.emit(key, false);
        }
    }
    else
    {
        if(m_keys[key] != state)
        {
            m_keys[key] = state;
            sig_key_released.emit(key, false);
        }
    }
}

void InputHandler::set_mouse_position(double x, double y)
{
    if(!m_mouse_moved_once)
    {
        m_mouse_last_x = x;
        m_mouse_last_y = y;
        m_mouse_moved_once = true;
        return;
    }

    m_mouse_delta_x = x - m_mouse_last_x;
    m_mouse_delta_y = m_mouse_last_y - y;

    m_mouse_last_x = x;
    m_mouse_last_y = y;

    sig_mouse_moved.emit(m_mouse_delta_x, m_mouse_delta_y, false);
}

void InputHandler::set_mouse_button_state(int button, bool state)
{
    m_mouse_buttons[button] = state;
    sig_mouse_state_changed.emit(button, state, m_mouse_last_x, m_mouse_last_y, false);
}

void InputHandler::poll_keyboard_input()
{
    for(size_t i = 0; i < m_keys.size(); i++)
    {
        int state = glfwGetKey(m_handle, i);
        set_key_state(i, static_cast<bool>(state));
    }
}

void InputHandler::poll_mouse_input()
{
    double x, y;
    glfwGetCursorPos(m_handle, &x, &y);
    set_mouse_position(x, y);

    for(size_t i = 0; i < m_mouse_buttons.size(); i++)
    {
        int state = glfwGetMouseButton(m_handle, i);
        set_mouse_button_state(i, static_cast<bool>(state));
    }
}

const std::array<bool, 1024> &InputHandler::keys()
{
    return m_keys;
}

bool InputHandler::key_state(int key)
{
    return m_keys[key];
}

Signal<int> &InputHandler::key_pressed()
{
    return sig_key_pressed;
}

Signal<int> &InputHandler::key_just_pressed()
{
    return sig_key_just_pressed;
}

Signal<int> &InputHandler::key_released()
{
    return sig_key_released;
}

Signal<double, double> &InputHandler::mouse_moved()
{
    return sig_mouse_moved;
}

Signal<int, bool, double, double> &InputHandler::mouse_button_state_changed()
{
    return sig_mouse_state_changed;
}

int InputHandler::mouse_x()
{
    return m_mouse_last_x;
}

int InputHandler::mouse_y()
{
    return m_mouse_last_y;
}

int InputHandler::mouse_dx()
{
    return m_mouse_delta_x;
}

int InputHandler::mouse_dy()
{
    return m_mouse_delta_y;
}

} // namespace nle
