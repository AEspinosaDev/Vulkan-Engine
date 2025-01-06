#include <engine/core/scene/scene.h>

void VKFW::Core::set_meshes(Scene* const scene, std::vector<Mesh*> meshes) {
    scene->m_meshes = meshes;
}
VKFW::Graphics::TLAS* VKFW::Core::get_TLAS(Scene* const scene) {
    return &scene->m_accel;
}
void VKFW::Core::Scene::compute_limits() {

    // for (Mesh* m : m_meshes)
    // {

    //     tricData* stats = g->get_properties();

    //     if (stats->maxCoords.x > maxCoords.x)
    //         maxCoords.x = stats->maxCoords.x;
    //     if (stats->maxCoords.y > maxCoords.y)
    //         maxCoords.y = stats->maxCoords.y;
    //     if (stats->maxCoords.z > maxCoords.z)
    //         maxCoords.z = stats->maxCoords.z;
    //     if (stats->minCoords.x < minCoords.x)
    //         minCoords.x = stats->minCoords.x;
    //     if (stats->minCoords.y < minCoords.y)
    //         minCoords.y = stats->minCoords.y;
    //     if (stats->minCoords.z < minCoords.z)
    //         minCoords.z = stats->minCoords.z;
    // }

    // center = (maxCoords + minCoords) * 0.5f;
    // radius = math::length((maxCoords - minCoords) * 0.5f);
}