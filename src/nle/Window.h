#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <any>
#include <string>
#include <atomic>
#include <stdexcept>
#include <functional>

#include "Utils.h"
#include "InputHandler.h"

#define NLE_WINDOW_DEFAULT_WIDTH    1280
#define NLE_WINDOW_DEFAULT_HEIGHT   720

namespace nle
{
    class Window
    {
    private:
        GLFWwindow *m_handle;
        InputHandler *m_input_handler;
        std::atomic_bool m_closed;
        std::function<void()> m_draw_callback;
        int m_width;
        int m_height;

        glm::vec2 m_mouse_last;
        glm::vec2 m_mouse_delta;
        bool m_mouse_moved_once;
        bool m_fullscreen;
        bool m_show_cursor;

        static void key_handler(GLFWwindow *window, int key, int code, int action, int mode);
        static void mouse_position_handler(GLFWwindow *window, double mouse_x, double mouse_y);
        static void mouse_button_handler(GLFWwindow *window, int button, int action, int mods);

    public:
        Window(GLuint w, GLuint h, const std::string &title);
        ~Window();
        void set_draw_callback(std::function<void()> dcb);
        void set_cursor_visibility(bool visible);
        bool cursor_visibility();
        void set_fullscreen(bool fullscreen);
        bool fullscreen();
        bool closed();
        void display();
        void close();
        InputHandler *input_handler();

        friend class Renderer3D;
    };
} // namespace nle

#endif