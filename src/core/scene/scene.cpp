#include <engine/core/scene/scene.h>

void VKFW::Core::set_meshes(Scene* const scene, std::vector<Mesh*> meshes) {
    scene->m_meshes = meshes;
}
VKFW::Graphics::TLAS* VKFW::Core::get_TLAS(Scene* const scene) {
    return &scene->m_accel;
}
void VKFW::Core::Scene::update_AABB() {

    m_volume.maxCoords = Vec3(0.0);
    m_volume.minCoords = Vec3(INFINITY);

    for (Mesh* m : m_meshes)
    {
        if (!m->is_active())
            continue;
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

    // Make a cube container for voxelization
    // float maxTerm      = std::max(std::max(m_volume.maxCoords.r, m_volume.maxCoords.g), m_volume.maxCoords.b);
    // float minTerm      = std::min(std::min(m_volume.minCoords.r, m_volume.minCoords.g), m_volume.minCoords.b);
    // m_volume.maxCoords = Vec3(maxTerm);
    // m_volume.minCoords = Vec3(minTerm);

    // Step 1: Compute bounding box center and extent
    Vec3 center = (m_volume.maxCoords + m_volume.minCoords) * 0.5f;
    Vec3 extent = (m_volume.maxCoords - m_volume.minCoords) * 0.5f;

    // Step 2: Compute the largest extent (to make it a cube)
    float halfSize = std::max({extent.x, extent.y, extent.z});

    // Step 3: Expand min and max symmetrically around center
    m_volume.minCoords = center - Vec3(halfSize);
    m_volume.maxCoords = center + Vec3(halfSize);

    m_volume.center = (m_volume.maxCoords + m_volume.minCoords) * 0.5f;
}