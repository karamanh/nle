#include "input_handler_glfw.h"

namespace nle
{
    input_handler_glfw::input_handler_glfw(GLFWwindow *handle)
        : input_handler(handle)
    {
    }

    void input_handler_glfw::poll_mouse_input()
    {
        if(!m_handle)
        {
            return;
        }

        double x, y;
        glfwGetCursorPos(m_handle, &x, &y);
        set_mouse_position(x, y);

        for(size_t i = 0; i < m_mouse_buttons.size(); i++)
        {
            int state = glfwGetMouseButton(m_handle, i);
            set_mouse_button_state(i, static_cast<bool>(state));
            set_mouse_position(x, y);
        }
    }

    void input_handler_glfw::poll_keyboard_input()
    {
        if(!m_handle)
        {
            return;
        }
        
        for(size_t i = 0; i < m_keys.size(); i++)
        {
            int state = glfwGetKey(m_handle, i);
            set_key_state(i, static_cast<bool>(state));
        }
    }
} // namespace nle
