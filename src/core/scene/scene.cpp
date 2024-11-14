#include <engine/core/scene/scene.h>

void VKFW::Core::set_meshes(Scene* const scene, std::vector<Mesh*> meshes) {
    scene->m_meshes = meshes;
}
VKFW::Graphics::TLAS* VKFW::Core::get_TLAS(Scene* const scene) {
    return &scene->m_accel;
}