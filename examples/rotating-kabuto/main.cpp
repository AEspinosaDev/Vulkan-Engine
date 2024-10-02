/*
    This file is part of the Vulkan-Engine demos folder. 
	In this folder you will find practical examples of different project setups.

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

	////////////////////////////////////////////////////////////////////////////////////

	Very simple demo of a kabuto rotating over and over again.

	////////////////////////////////////////////////////////////////////////////////////

*/
#include <iostream>
#include <engine/core.h>

USING_VULKAN_ENGINE_NAMESPACE

int main()
{

	try
	{
		float delta;
		float last{ 0 };

		Window* window = new Window("Kabuto", 800, 600);

		window->init();

		RendererSettings settings{};
		settings.AAtype = AntialiasingType::MSAA_x4;
		settings.clearColor = Vec4(0.0, 0.0, 0.0, 1.0);
		settings.renderingType = TFORWARD;

		Renderer* renderer = new Renderer(window, settings);

		Camera* camera = new Camera();
		camera->set_position(Vec3(0.0f, 0.15f, -1.0f));
		camera->set_far(10);
		camera->set_near(0.1f);
		camera->set_field_of_view(70.0f);

		Scene* scene = new Scene(camera);
		PointLight* light = new PointLight();
		light->set_position({ -3.0f, 3.0f, -1.0f });
		light->set_cast_shadows(false);
		scene->add(light);

		Mesh* kabuto = new Mesh();
		kabuto->load_file(EXAMPLES_RESOURCES_PATH "meshes/kabuto.obj", true);
		kabuto->set_scale(0.4f);

		PhysicallyBasedMaterial* material = new PhysicallyBasedMaterial();
		material->set_albedo(Vec4{ 1.0 });
		kabuto->set_material(material);

		scene->add(kabuto);

		const float DELTA_DEG = 45.0F;
		while (!window->get_window_should_close())
		{
			float currentTime = (float)Window::get_time_elapsed();
			delta = currentTime - last;
			last = currentTime;

			kabuto->set_rotation({ 0.0f,kabuto->get_rotation().y + DELTA_DEG * delta,0.0f }, false);

			Window::poll_events();
			renderer->render(scene);
		}
		renderer->shutdown(scene);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}