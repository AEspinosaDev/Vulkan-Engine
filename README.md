 <H1 ALIGN="CENTER"> VULKAN 3D LIBRAY 👀 </H1>

  ![Build Status](https://github.com/AEspinosaDev/Vulkan-Engine/actions/workflows/build-windows.yml/badge.svg)

<p align="center"> 
A Vulkan based 3D library that acts as a wrapper for the most verbose Vulkan parts that I implemented to learn the API.
	
![Screenshot (76)](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/6240417a-d655-4bb5-a0d5-03406a71668a)
![Screenshot (85)](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/c3f468cc-10e4-456f-86bc-eb4f0a6d0c05)

<p align="center">
  <img src="https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/3d926089-8889-4e7f-8cab-109cf7ff17b3" alt="Screenshot (82)" width="33%" />
  <img src="https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/a99ce87b-1eac-4148-969d-ff0fd622d684" alt="Screenshot (81)" width="33%" />
  <img src="https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/ae46f154-31e8-45e4-8c0a-51d6a483c89d" alt="Screenshot (80)" width="33%" />
</p>




## Project Structure 🗃️

It consists of two parts:

- The library itself, a place where you can find a lot of abstracted functionality.

- An examples directory that links against the library where you can find useful examples.

The main feautures of the library are:

- Forward and Deferred pipelines support.
- PBR, Phong and other types of materials abstractions.
- Scene and scene objects abstractions.
- On the fly shader compiling.
- Raytracing Support (Shadows, AO).
- Texture loading with mipmapping.
- Dynamic renderer states for some graphic features.
- Multipass (depth pass for shadows, SSAO and post-processing).
- Vulkan object and functionality abstraction.
- IBL.
- Simple to use user interface.
- Easy to distribute source code.

This project is a work in progress. It has a long way until becoming a decent library, there are no fancy features for now, but all the basics are here. Eventually, with time, I will be adding more advanced functionality.

## Project Building 🛠️

The prequisites for using this code are:

- Windows 10, 11 (Although it should be easy enough to set it up for Linux).
- Vulkan SDK 1.3.* installed. (With VMA and Shaderc libraries)
- CMake installed.

1. Clone the repo:
   ```bash
   git clone --recursive https://github.com/AEspinosaDev/Vulkan-Engine.git
   cd Vulkan-Engine
   ```
2. Build with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

The project is configured in such a way that, during the build process, CMake takes care of automatically locating and linking all dependencies on the system, with exception of the Vulkan SDK, due to its more complex installation. This has been done to facilitate an easy and lightweight distribution of the source code, sparing the user the effort of manual configuration. Although the project has been implemented in Visual Studio Code, a practical file structure has been configured for CMake in case it is opened in Visual Studio.

Once the project is opened in the IDE of choice, compile it in the desired mode, and it would be ready to run. The CMake configuration is set for a 64-bit architecture, but it can be changed. CMake also takes care of automatically configuring the paths for resource files.

The project compiles dependencies, the 3D library, and the example applications directory, which statically links against the 3D library. The library is a STATIC lib, do not try to link dynamically against it.

3. Building of the demos directory is optional, and can be turned off in CMake:
```bash
cmake -DBUILD_EXAMPLES=OFF /path/to/source
```
4. Alternatively, you can click on the build.bat file to automatically build (in release mode) the entire project.

## Project Integration 🗄️

Integration of Vulkan-Engine into your own personal project is quite easy. If working with CMake, Vulkan-Engine should be inserted inside the dependencies folder of your project root directory.

On your main project CMakeLists.txt, write:

```cmake

cmake_minimum_required(VERSION 3.16)

project(JohnDoe VERSION 1.0.0)

# Use C++17 (Otherwise it won't work)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#check SDK
find_package(Vulkan REQUIRED)
get_filename_component(VULKAN_SDK_ROOT ${Vulkan_LIBRARY} DIRECTORY)

# Add Vulkan-Engine subdirectory ! Set if you want to build the examples directory ...
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(dependencies/Vulkan-Engine)
# Important step, link vulkan sdf directories to project root
link_directories(${VULKAN_SDK_ROOT})

#Setup project own source code (User-Defined)
file(GLOB APP_SOURCES "src/*.cpp")
file(GLOB APP_HEADERS "include/*.h")
add_executable(JohnDoe ${APP_SOURCES} ${APP_HEADERS})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Link project against VulkanEngine
target_link_libraries(JohnDoe PRIVATE VulkanEngine)

....

```

Your project structure should be somewhat like this one:

```
project_root/
│
├── resources/
│   ├── shaders
│   ├── textures
│   └── meshes
│
├── dependencies/
│   ├── Vulkan-Engine
│   └── another-dependency
│
├── src/
│   ├── main.cpp
│   └── compilation_file1.cpp
│
├── include/
│   ├── header1.h
│   └── header2.h
│
└── CMakeLists.txt
````

## Project Usage ✨

Here is a simple snippet for creating a basic scene:

```cpp
#include <engine/core.h>
#include <engine/systems.h>
#include <engine/tools/loaders.h>
#include <iostream>

USING_VULKAN_ENGINE_NAMESPACE

int main()
{

    try
    {
        float delta;
        float last{0};

        // Setup a window
        Core::IWindow *window = new Core::WindowGLFW("Kabuto", 800, 600);
        window->init();

        // Create the renderer, you can play with the settings here
        Systems::RendererSettings settings{};
        settings.samplesMSAA = MSAASamples::x4;
        settings.clearColor = Vec4(0.0, 0.0, 0.0, 1.0);
        Systems::BaseRenderer *renderer = new Systems::ForwardRenderer(window, settings, {});

        // Create a camera
        Core::Camera *camera = new Core::Camera();
        camera->set_position(Vec3(0.0f, 0.15f, -1.0f));
        camera->set_far(10);
        camera->set_near(0.1f);
        camera->set_field_of_view(70.0f);

        // Create a scene and fill it with a light and a model
        Core::Scene *scene = new Core::Scene(camera);
        Core::PointLight *light = new Core::PointLight();
        light->set_position({-3.0f, -3.0f, -1.0f});
        light->set_cast_shadows(false);
        scene->add(light);

        Core::Mesh *model = new Core::Mesh();
        Tools::Loaders::load_3D_file(model, ENGINE_RESOURCES_PATH "meshes/cube.obj");
        model->set_scale(0.4f);
        model->set_rotation({45.0f, 45.0f, 0.0f});

        Core::PhysicallyBasedMaterial *material = new Core::PhysicallyBasedMaterial();
        material->set_albedo(Vec4{1.0});
        model->push_material(material);

        scene->add(model);

        // Rendering loop by quering the window obj
        while (!window->get_window_should_close())
        {
            window->poll_events();
            renderer->render(scene);
        }
        // Call this function conviniently shut down the renderer
        // By doing this, the renderer will clean all memory used by its resources
        renderer->shutdown(scene);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

The code provided generates this output:

<img src="https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/35755111-00af-43f6-a940-06a9ae60e1e9" alt="image" width="75%" height="75%">

With a little extra effort, you can create much richer and interactive applications:

![vulkan](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/58e12bf5-a5d3-4d9f-8a27-33de309a5fff)

As you can see, the library is easy to use: with a window, a camera, a scene filled with some meshes and of course a renderer, you have everything you need to start working. 


