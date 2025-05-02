/*
    This file is part of the Vulkan-Engine demos folder.
    In this folder you will find practical examples of different project setups.

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

    ////////////////////////////////////////////////////////////////////////////////////

    Very simple demo of a kabuto rotating over and over again.

    ////////////////////////////////////////////////////////////////////////////////////

*/
#include <engine/core.h>
#include <engine/systems.h>
#include <engine/tools/loaders.h>
#include <iostream>

USING_VULKAN_ENGINE_NAMESPACE

int main() {

    try
    {
        float delta;
        float last{0};

        ptr<Core::IWindow> window = std::make_shared<Core::WindowGLFW>("Kabuto", 800, 600);

        window->init();

        Systems::RendererSettings settings{};
        settings.samplesMSAA = MSAASamples::x1;
        settings.softwareAA = SoftwareAA::FXAA;
        settings.clearColor  = Vec4(0.0, 0.0, 0.0, 1.0);

        ptr<Systems::BaseRenderer> renderer = std::make_shared<Systems::ForwardRenderer>(window, settings);

        Core::Camera* camera = new Core::Camera();
        camera->set_position(Vec3(0.0f, 0.15f, -1.0f));
        camera->set_far(10);
        camera->set_near(0.1f);
        camera->set_field_of_view(70.0f);

        Core::Scene*      scene = new Core::Scene(camera);
        Core::PointLight* light = new Core::PointLight();
        light->set_position({-3.0f, 3.0f, -1.0f});
        light->set_cast_shadows(false);
        scene->add(light);

        Core::Mesh* kabuto = new Core::Mesh();
        Tools::Loaders::load_3D_file(kabuto, EXAMPLES_RESOURCES_PATH "meshes/kabuto.obj");
        kabuto->set_scale(0.4f);

        Core::PhysicalMaterial* material = new Core::PhysicalMaterial();
        material->set_albedo(Vec4{1.0});
        kabuto->push_material(material);

        scene->add(kabuto);

        const float DELTA_DEG = 45.0F;
        while (!window->get_window_should_close())
        {
            float currentTime = (float)window->get_time_elapsed();
            delta             = currentTime - last;
            last              = currentTime;

            kabuto->set_rotation({0.0f, kabuto->get_rotation().y + DELTA_DEG * delta, 0.0f}, false);

            window->poll_events();
            renderer->render(scene);
        }
        renderer->shutdown(scene);
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}