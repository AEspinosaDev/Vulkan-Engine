#ifndef SAMPLE_APP_H
#define SAMPLE_APP_H

#include <chrono>

#include <engine/core.h>

#include <engine/utilities/controller.h>
#include <engine/utilities/gui.h>
#include <engine/utilities/renderer_widget.h>
#include <engine/utilities/loaders.h>

/**
 * Example app
 */
USING_VULKAN_ENGINE_NAMESPACE

class VulkanRenderer
{
    struct UserInterface
    {
        GUIOverlay *overlay{nullptr};

        Panel *explorer{nullptr};
        Panel *tutorial{nullptr};
        Panel *properties{nullptr};
        SceneExplorerWidget *scene{nullptr};
        ObjectExplorerWidget *object{nullptr};
    };
    UserInterface m_interface{};

    Window *m_window;
    Renderer *m_renderer;
    Scene *m_scene;
    Camera *camera;
    Controller *m_controller;

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
    void
    init(RendererSettings settings, ForwardRendererSettings settings2);

    void run(int argc, char* argv[]);

private:
    void setup();

    void setup_gui();

    void tick();

    void update();

#pragma region Input Management

    void keyboard_callback(int key, int scancode, int action, int mods)
    {

        if (glfwGetKey(m_window->get_handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            m_window->set_window_should_close(true);
        }

        if (glfwGetKey(m_window->get_handle(), GLFW_KEY_F11) == GLFW_PRESS)
        {
            m_window->set_fullscreen(m_window->is_fullscreen() ? false : true);
        }
        if (glfwGetKey(m_window->get_handle(), GLFW_KEY_L) == GLFW_PRESS)
        {
            animateLight = animateLight ? false : true;
        }
    }

    void mouse_callback(double xpos, double ypos)
    {
        if (m_interface.overlay->wants_to_handle_input())
            return;

        m_controller->handle_mouse(m_window->get_handle(), (float)xpos, (float)ypos);
    }

    void window_resize_callback(int width, int height)
    {
        m_window->set_size(width, height);
        m_interface.overlay->set_extent({width, height});
    }

#pragma endregion
};

#endif