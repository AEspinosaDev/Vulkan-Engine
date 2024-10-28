#ifndef LIGHT_APP_H
#define LIGHT_APP_H

#include <chrono>

#include <engine/core.h>
#include <engine/systems.h>

#include <engine/tools/controller.h>
#include <engine/tools/gui.h>
#include <engine/tools/loaders.h>
#include <engine/tools/renderer_widget.h>

/**
 * Example app
 */
USING_VULKAN_ENGINE_NAMESPACE
using namespace Core;
class VulkanRenderer
{
    struct UserInterface
    {
        Tools::GUIOverlay *overlay{nullptr};
        Tools::Panel *explorer{nullptr};
        Tools::Panel *tutorial{nullptr};
        Tools::Panel *properties{nullptr};
        Tools::SceneExplorerWidget *scene{nullptr};
        Tools::ObjectExplorerWidget *object{nullptr};
    };
    UserInterface m_interface{};

    IWindow *m_window;
    Systems::BaseRenderer *m_renderer;
    Scene *m_scene;
    Camera *camera;
    Tools::Controller *m_controller;

    Mesh *m_lightDummy;
    bool animateLight{true};

    struct Time
    {
        float delta{0.0f};
        float last{0.0f};
        float framesPerSecond{0.0f};
    };
    Time m_time{};

  public:
    void init(Systems::RendererSettings settings);

    void run(int argc, char *argv[]);

  private:
    void setup();

    void setup_gui();

    void tick();

    void update();

#pragma region Input Management

    void keyboard_callback(int key, int scancode, int action, int mods)
    {
        void *windowHandle{nullptr};
        m_window->get_handle(windowHandle);
        GLFWwindow *glfwWindow = static_cast<GLFWwindow *>(windowHandle);
        if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            m_window->set_window_should_close(true);
        }

        if (glfwGetKey(glfwWindow, GLFW_KEY_F11) == GLFW_PRESS)
        {
            m_window->set_fullscreen(m_window->is_fullscreen() ? false : true);
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_L) == GLFW_PRESS)
        {
            animateLight = animateLight ? false : true;
        }
    }

    void mouse_callback(double xpos, double ypos)
    {
        if (m_interface.overlay->wants_to_handle_input())
            return;

        m_controller->handle_mouse((float)xpos, (float)ypos);
    }

    void window_resize_callback(int width, int height)
    {
        m_window->set_size(width, height);
        m_interface.overlay->set_extent({width, height});
    }

#pragma endregion
};

#endif