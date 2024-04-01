 <H1 ALIGN="CENTER"> VULKAN 3D LIBRAY 👀 </H1>
<p align="center"> 
A Vulkan based 3D library that acts as a wrapper for the most verbose Vulkan parts that I implemented to learn the API.

![Screenshot (51)](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/89759b3c-b129-4f54-b177-8b9c7ac3b14c)
![Screenshot (52)](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/32acad6d-4ddb-4952-a877-08e8a20fdf7e)

## Project Structure 🗃️

It consists of two parts:

- The library itself, a place where you can find a lot of abstracted functionality.

- A demos directory that links against the library where you can find useful examples.

The main feautures of the library are:

- PBR, Phong and other types of materials abstractions.
- Scene and scene objects abstractions.
- Texture loading with mipmapping.
- On the fly shader compiling.
- Dynamic renderer states for some graphic features.
- Multipass (depth pass for shadows and SSAO).
- Vulkan object and functionality abstraction.
- Simple to use user interface (Unfinished).
- Easy to distribute source code.

This project is a work in progress. It has a long way until becoming a decent library, there are no fancy features for now, but all the basics are here. Eventually, with time, I will be adding more advanced functionality.

## Project building and usage 🛠️

The prequisites for using this code are:

- Windows 10, 11 (Although it should be easy enough to set it up for Linux).
- Vulkan SDK 1.3.* installed.
- CMake installed.

1. Clone the repo:
   ```bash
   git clone https://github.com/AEspinosaDev/Vulkan-Engine.git
   cd Vulkan-Engine
   ```
2. Build with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```
The project is configured in such a way that, during the build process, CMake takes care of automatically locating and linking all dependencies on the system, as well as importing them from the internet if they are not available, with exception of the Vulkan SDK, due to its more complex installation. This has been done to facilitate an easy and lightweight distribution of the source code, sparing the user the effort of manual configuration. Although the project has been implemented in Visual Studio Code, a practical file structure has been configured for CMake in case it is opened in Visual Studio.

Once the project is opened in the IDE of choice, compile it in the desired mode, and it would be ready to run. The CMake configuration is set for a 64-bit architecture, but it can be changed. CMake also takes care of automatically configuring the paths for resource files.

The project compiles dependencies, the 3D library, and the demonstration applications directory, which statically links against the 3D library. The library is a STATIC lib, do not try to link dynamically against it.

3. Building of the demos directory is optional, and can be turn off in CMake:
```bash
cmake -DBUILD_DEMOS=OFF /path/to/source
```

4. Here is a simple snippet for creating a basic scene:

```cpp
#include <iostream>
#include <engine/vk_renderer.h>

USING_VULKAN_ENGINE_NAMESPACE

int main()
{

	try
	{
  //Get sample meshes path from the engine to easy access them
		const std::string MODEL_PATH(VK_MODEL_DIR);

  //Setup the window
		Window* window = new Window("Example", 800, 600);
		window->init();

  //Create the renderer, you can play with the settings here
		RendererSettings settings{};
		settings.AAtype = AntialiasingType::MSAA_x4;
		settings.clearColor = Vec4(0.0, 0.0, 0.0, 1.0);
		Renderer* renderer = new Renderer(window, settings);

  //Create the camera
		Camera* camera = new Camera();
		camera->set_position(Vec3(0.0f, 0.15f, -1.0f));
		camera->set_far(10);
		camera->set_near(0.1f);
		camera->set_field_of_view(65.0f);

  //Create the scene and fill it with a light and a model
		Scene* scene = new Scene(camera);
		PointLight* light = new PointLight();
		light->set_position({ -3.0f, -3.0f, -1.0f });
		light->set_cast_shadows(false);
		scene->add(light);

		Mesh* model = new Mesh();
		model->load_file(MODEL_PATH + "cube.obj", true); //Call the model path and 
		model->set_scale(0.4f);
		model->set_rotation({ 45.0f,45.0f,0.0f });

		PhysicallyBasedMaterial* material = new PhysicallyBasedMaterial();
		material->set_albedo(Vec4{ 1.0 });
		model->set_material(material);

		scene->add(model);

  //Rendering loop by quering the window obj
		while (!window->get_window_should_close())
		{
			Window::poll_events();
			renderer->render(scene);
		}

		renderer->shutdown();

	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
```

As you can see, the library is easy to use: with a window, a camera, a scene filled with some meshes and of course a renderer, you have everything you need to start working. 


![vulkan](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/58e12bf5-a5d3-4d9f-8a27-33de309a5fff)
