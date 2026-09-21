/**
 * @file particle_test.cpp
 * @brief That the particle shader compiles, links, and draws what it says.
 *
 * A shader that will not compile draws nothing and says nothing about it --
 * the program is zero, every draw is quietly dropped, and the first anyone
 * knows is an empty screen where the fire should be. That is exactly how the
 * first version of this shipped, so it is now something a test can catch
 * rather than something a person has to notice.
 *
 * Needs a GL context, and makes a hidden one. Without a display it reports
 * itself skipped rather than failed: a machine with no GPU has not broken
 * anything.
 */

#include "nle/renderer/particles.h"
#include "nle/renderer/render_command.h"
#include "nle/renderer/opengl_backend.h"
#include "nle/renderer/render_context.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

/// Drains and reports anything GL has been holding against us.
bool gl_is_quiet(const std::string& after)
{
    bool quiet = true;

    for(GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
    {
        std::cout << "         GL error 0x" << std::hex << error << std::dec
                  << " after " << after << "\n";
        quiet = false;
    }

    return quiet;
}

void test_it_draws()
{
    std::cout << "\nparticles\n";

    auto emitter = nle::make_ref<nle::particle_emitter>();

    emitter->set_style(nle::particle_style::flame);
    emitter->set_count(64);
    emitter->set_colours(glm::vec3(1.0f, 0.6f, 0.2f), glm::vec3(0.5f, 0.1f, 0.0f));

    nle::render_command_buffer commands;
    nle::render_context context;

    context.view = glm::mat4(1.0f);
    context.projection = glm::mat4(1.0f);
    context.delta_time = 1.0f / 60.0f;

    gl_is_quiet("setting up");

    // Recording is where the shader and the quad are made, the first time.
    emitter->render(commands, context);

    check(gl_is_quiet("recording a frame of particles"),
          "building the shader and the quad upsets nothing");

    check(commands.commands().size() > 0, "and something was recorded");

    // The draw itself. A shader that did not link has program zero, and this
    // is where that shows up as an error rather than as an empty screen.
    nle::opengl_backend backend;

    backend.begin_frame(context);
    backend.execute_commands(commands);

    check(gl_is_quiet("drawing them"), "and drawing them upsets nothing either");

    // Every style, since each is a different branch of the same shader and a
    // mistake in one would never be reached by testing another.
    for(const auto style : { nle::particle_style::flame, nle::particle_style::cloud,
                             nle::particle_style::spark })
    {
        emitter->set_style(style);

        commands.clear();
        emitter->render(commands, context);

        backend.begin_frame(context);
        backend.execute_commands(commands);
    }

    check(gl_is_quiet("drawing every style"), "and so does every style of them");

    // An emitter of nothing should record nothing rather than a draw of zero
    // instances, which some drivers treat as an error.
    emitter->set_count(0);

    commands.clear();
    emitter->render(commands, context);

    check(commands.commands().empty(), "an emitter of no particles records no work");
}

} // namespace

int main()
{
    if(!glfwInit())
    {
        std::cerr << "could not initialise glfw\n";
        return 77; // treated as "skipped" by ctest
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(64, 64, "nle particle test", nullptr, nullptr);

    if(!window)
    {
        std::cerr << "could not create a hidden gl context\n";
        glfwTerminate();
        return 77;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if(glewInit() != GLEW_OK)
    {
        std::cerr << "could not initialise glew\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 77;
    }

    // glewInit leaves an error behind on a core profile. Not ours.
    glGetError();

    test_it_draws();

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
