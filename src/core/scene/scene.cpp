#include <engine/core/scene/scene.h>

void VKFW::Core::set_meshes(Scene* const scene, std::vector<Mesh*> meshes) {
    scene->m_meshes = meshes;
}
VKFW::Graphics::TLAS* VKFW::Core::get_TLAS(Scene* const scene) {
    return &scene->m_accel;
}
void VKFW::Core::Scene::update_AABB() {

    for (Mesh* m : m_meshes)
    {
        BV const* bvolume = m->get_bounding_volume();
        if (!bvolume)
            continue;

        Mat4 modelMat  = m->get_model_matrix();
        Vec3 minCoords = modelMat * Vec4(bvolume->minCoords, 1.f);
        Vec3 maxCoords = modelMat * Vec4(bvolume->maxCoords, 1.f);

        /*MAX*/
        if (maxCoords.x > m_volume.maxCoords.x)
            m_volume.maxCoords.x = maxCoords.x;
        if (maxCoords.y > m_volume.maxCoords.y)
            m_volume.maxCoords.y = maxCoords.y;
        if (maxCoords.z > m_volume.maxCoords.z)
            m_volume.maxCoords.z = maxCoords.z;
        /*MIN*/
        if (minCoords.x < m_volume.minCoords.x)
            m_volume.minCoords.x = minCoords.x;
        if (minCoords.y < m_volume.minCoords.y)
            m_volume.minCoords.y = minCoords.y;
        if (minCoords.z < m_volume.minCoords.z)
            m_volume.minCoords.z = minCoords.z;
    }

    m_volume.center = (m_volume.maxCoords + m_volume.minCoords) * 0.5f;
}