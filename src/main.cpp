#include <iostream>

#include "gfx/vk_renderer.h"

int main()
{

    vkeng::Renderer app(new vkeng::Window("VK", 800, 600));

    try
    {
        app.init();

        std::vector<vkeng::Mesh *> meshes;
        vkeng::Mesh *m = vkeng::Mesh::load({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}},
                                           {0, 1, 2, 2, 3, 0});
        vkeng::Mesh *m2 = vkeng::Mesh::load({{{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
                                             {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                             {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}});

        meshes.push_back(m);

        vkeng::Camera *camera = new vkeng::Camera();

        camera->set_position(glm::vec3(0.0f,0.0f,-1.0f));
        camera->set_far(200.0f);
        camera->set_near(0.01f);
        camera->set_field_of_view(70.0f);


        // app.render(meshes,camera);
        app.run(meshes,camera);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}