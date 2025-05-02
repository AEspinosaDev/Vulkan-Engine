#include <engine/core/scene/mesh.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {
int Mesh::m_instanceCount = 0;

void BoundingSphere::setup(Mesh* const mesh) {
    maxCoords = {0.0f, 0.0f, 0.0f};
    minCoords = {INFINITY, INFINITY, INFINITY};

    auto                 g     = mesh->get_geometry();
    const GeometricData& stats = g->get_properties();

    if (stats.maxCoords.x > maxCoords.x)
        maxCoords.x = stats.maxCoords.x;
    if (stats.maxCoords.y > maxCoords.y)
        maxCoords.y = stats.maxCoords.y;
    if (stats.maxCoords.z > maxCoords.z)
        maxCoords.z = stats.maxCoords.z;
    if (stats.minCoords.x < minCoords.x)
        minCoords.x = stats.minCoords.x;
    if (stats.minCoords.y < minCoords.y)
        minCoords.y = stats.minCoords.y;
    if (stats.minCoords.z < minCoords.z)
        minCoords.z = stats.minCoords.z;

    center = (maxCoords + minCoords) * 0.5f;
    radius = math::length((maxCoords - minCoords) * 0.5f);
}

bool BoundingSphere::is_on_frustrum(const Frustum& frustum) const

{
    const Vec3 globalScale = obj->get_scale();

    const Vec3 globalCenter{obj->get_model_matrix() * Vec4(center, 1.f)};

    const float maxScale     = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);
    const float globalRadius = radius * maxScale;

    return (frustum.leftFace.get_signed_distance(globalCenter) >= -globalRadius && frustum.rightFace.get_signed_distance(globalCenter) >= -globalRadius &&
            frustum.farFace.get_signed_distance(globalCenter) >= -globalRadius && frustum.nearFace.get_signed_distance(globalCenter) >= -globalRadius &&
            frustum.topFace.get_signed_distance(globalCenter) >= -globalRadius && frustum.bottomFace.get_signed_distance(globalCenter) >= -globalRadius);
}
void AABB::setup(Mesh* const mesh) {
}
bool AABB::is_on_frustrum(const Frustum& frustum) const {
    return false;
}

IMaterial* Mesh::change_material(IMaterial* m, size_t id) {
    if (m_material.size() < id + 1)
    {
        ERR_LOG("Not enough material slots");
        return nullptr;
    }

    IMaterial* old_m = m_material[id];
    m_material[id]   = m;
    return old_m;
}
void Mesh::setup_volume(VolumeType type) {
    if (m_volume)
        delete m_volume;

    switch (type)
    {
    case VolumeType::SPHERE_VOLUME:
        m_volume = new BoundingSphere(this);
        break;

    case VolumeType::AABB_VOLUME:
        m_volume = new AABB(this);
        break;
    default:
        break;
    }

    m_volume->setup(this);
}

Mesh* Mesh::clone() const {

    Mesh* mesh       = new Mesh();
    mesh->m_material = m_material;
    mesh->m_geometry = m_geometry;
    mesh->setup_volume();
    mesh->set_name(m_name + std::string(" clone"));
    mesh->set_transform(m_transform);
    m_instanceCount++;
    return mesh;
}

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END