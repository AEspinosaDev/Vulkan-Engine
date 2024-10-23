#include <engine/core/scene/scene.h>

void VKFW::Core::set_meshes(Scene * const scene, std::vector<Mesh*> meshes)
{
    scene->m_meshes = meshes;
}