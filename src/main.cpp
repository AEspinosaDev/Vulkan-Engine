#include <iostream>

#include "gfx/vk_renderer.h"



int main() {

    vkeng::Renderer app(new vkeng::Window("VK", 800, 600));
    
    try {
        app.init();

        std::vector<vkeng::Mesh*> meshes;
        vkeng::Mesh* m = vkeng::Mesh::load();
        vkeng::Mesh* m2 = vkeng::Mesh::load2();
        meshes.push_back(m);
        meshes.push_back(m2);
        
        app.run(meshes);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}