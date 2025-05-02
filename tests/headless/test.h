#pragma once

#include <chrono>

#include <engine/core.h>
#include <engine/systems.h>

#include <engine/tools/controller.h>
#include <engine/tools/gui.h>
#include <engine/tools/loaders.h>

/**
 * Headless app
 */
USING_VULKAN_ENGINE_NAMESPACE
using namespace Core;
class Application
{

    ptr<Systems::BaseRenderer> m_renderer;
    Scene*                 m_scene;
    Camera*                m_camera;

  public:
    void init(Systems::RendererSettings settings);

    void run(int argc, char* argv[]);

  private:
    void setup();
};
