/**
 * @file input_handler_glfw.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-08-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "input_handler.hpp"
#include <GLFW/glfw3.h>

namespace nle
{

class input_handler_glfw : public input_handler<GLFWwindow*>
{
public:
    enum class key
    {
        unknown = -1,
        space = GLFW_KEY_SPACE,
        apostrophe = GLFW_KEY_APOSTROPHE,
        comma = GLFW_KEY_COMMA,
        minus = GLFW_KEY_MINUS,
        period = GLFW_KEY_PERIOD,
        slash = GLFW_KEY_SLASH,
        num0 = GLFW_KEY_0,
        num1 = GLFW_KEY_1,
        num2 = GLFW_KEY_2,
        num3 = GLFW_KEY_3,
        num4 = GLFW_KEY_4,
        num5 = GLFW_KEY_5,
        num6 = GLFW_KEY_6,
        num7 = GLFW_KEY_7,
        num8 = GLFW_KEY_8,
        num9 = GLFW_KEY_9,
        semicolon = GLFW_KEY_SEMICOLON,
        equal = GLFW_KEY_EQUAL,
        a = GLFW_KEY_A,
        b = GLFW_KEY_B,
        c = GLFW_KEY_C,
        d = GLFW_KEY_D,
        e = GLFW_KEY_E,
        f = GLFW_KEY_F,
        g = GLFW_KEY_G,
        h = GLFW_KEY_H,
        i = GLFW_KEY_I,
        j = GLFW_KEY_J,
        k = GLFW_KEY_K,
        l = GLFW_KEY_L,
        m = GLFW_KEY_M,
        n = GLFW_KEY_N,
        o = GLFW_KEY_O,
        p = GLFW_KEY_P,
        q = GLFW_KEY_Q,
        r = GLFW_KEY_R,
        s = GLFW_KEY_S,
        t = GLFW_KEY_T,
        u = GLFW_KEY_U,
        v = GLFW_KEY_V,
        w = GLFW_KEY_W,
        x = GLFW_KEY_X,
        y = GLFW_KEY_Y,
        z = GLFW_KEY_Z,
        left_bracket = GLFW_KEY_LEFT_BRACKET, // '['
        backslash = GLFW_KEY_BACKSLASH,       // '\'
        right_bracket = GLFW_KEY_RIGHT_BRACKET, // ']'
        grave_accent = GLFW_KEY_GRAVE_ACCENT, // '`'
        world_1 = GLFW_KEY_WORLD_1, // non-US #1
        world_2 = GLFW_KEY_WORLD_2, // non-US #2
        
        escape = GLFW_KEY_ESCAPE,
        enter = GLFW_KEY_ENTER,
        tab = GLFW_KEY_TAB,
        backspace = GLFW_KEY_BACKSPACE,
        insert = GLFW_KEY_INSERT,
        delete_key = GLFW_KEY_DELETE,
        right = GLFW_KEY_RIGHT,
        left = GLFW_KEY_LEFT,
        down = GLFW_KEY_DOWN,
        up = GLFW_KEY_UP,
        page_up = GLFW_KEY_PAGE_UP,
        page_down = GLFW_KEY_PAGE_DOWN,
        home = GLFW_KEY_HOME,
        end = GLFW_KEY_END,
        caps_lock = GLFW_KEY_CAPS_LOCK,
        scroll_lock = GLFW_KEY_SCROLL_LOCK,
        num_lock = GLFW_KEY_NUM_LOCK,
        print_screen = GLFW_KEY_PRINT_SCREEN,
        pause = GLFW_KEY_PAUSE,
        f1 = GLFW_KEY_F1,
        f2 = GLFW_KEY_F2,
        f3 = GLFW_KEY_F3,
        f4 = GLFW_KEY_F4,
        f5 = GLFW_KEY_F5,
        f6 = GLFW_KEY_F6,
        f7 = GLFW_KEY_F7,
        f8 = GLFW_KEY_F8,
        f9 = GLFW_KEY_F9,
        f10 = GLFW_KEY_F10,
        f11 = GLFW_KEY_F11,
        f12 = GLFW_KEY_F12,
        f13 = GLFW_KEY_F13,
        f14 = GLFW_KEY_F14,
        f15 = GLFW_KEY_F15,
        f16 = GLFW_KEY_F16,
        f17 = GLFW_KEY_F17,
        f18 = GLFW_KEY_F18,
        f19 = GLFW_KEY_F19,
        f20 = GLFW_KEY_F20,
        f21 = GLFW_KEY_F21,
        f22 = GLFW_KEY_F22,
        f23 = GLFW_KEY_F23,
        f24 = GLFW_KEY_F24,
        f25 = GLFW_KEY_F25,
        keypad_0 = GLFW_KEY_KP_0,
        keypad_1 = GLFW_KEY_KP_1,
        keypad_2 = GLFW_KEY_KP_2,
        keypad_3 = GLFW_KEY_KP_3,
        keypad_4 = GLFW_KEY_KP_4,
        keypad_5 = GLFW_KEY_KP_5,
        keypad_6 = GLFW_KEY_KP_6,
        keypad_7 = GLFW_KEY_KP_7,
        keypad_8 = GLFW_KEY_KP_8,
        keypad_9 = GLFW_KEY_KP_9,
        keypad_decimal = GLFW_KEY_KP_DECIMAL,
        keypad_divide = GLFW_KEY_KP_DIVIDE,
        keypad_multiply = GLFW_KEY_KP_MULTIPLY,
        keypad_subtract = GLFW_KEY_KP_SUBTRACT,
        keypad_add = GLFW_KEY_KP_ADD,
        keypad_enter = GLFW_KEY_KP_ENTER,
        keypad_equal = GLFW_KEY_KP_EQUAL,
        left_shift = GLFW_KEY_LEFT_SHIFT,
        left_control = GLFW_KEY_LEFT_CONTROL,
        left_alt = GLFW_KEY_LEFT_ALT,
        left_super = GLFW_KEY_LEFT_SUPER,
        right_shift = GLFW_KEY_RIGHT_SHIFT,
        right_control = GLFW_KEY_RIGHT_CONTROL,
        right_alt = GLFW_KEY_RIGHT_ALT,
        right_super = GLFW_KEY_RIGHT_SUPER,
        menu = GLFW_KEY_MENU
    };

    input_handler_glfw(GLFWwindow *handle);
    
    void poll_mouse_input() override;

    void poll_keyboard_input() override;
};

} // namespace nle
