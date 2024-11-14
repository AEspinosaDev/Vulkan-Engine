#include <engine/core/scene/mesh.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core
{
int Mesh::m_instanceCount = 0;

void Sphere::setup(Mesh *const mesh)
{
    Vec3 maxCoords = {0.0f, 0.0f, 0.0f};
    Vec3 minCoords = {INFINITY, INFINITY, INFINITY};

    for (Geometry *g : mesh->get_geometries())
    {
        const GeometricData *stats = g->get_properties();

        if (stats->maxCoords.x > maxCoords.x)
            maxCoords.x = stats->maxCoords.x;
        if (stats->maxCoords.y > maxCoords.y)
            maxCoords.y = stats->maxCoords.y;
        if (stats->maxCoords.z > maxCoords.z)
            maxCoords.z = stats->maxCoords.z;
        if (stats->minCoords.x < minCoords.x)
            minCoords.x = stats->minCoords.x;
        if (stats->minCoords.y < minCoords.y)
            minCoords.y = stats->minCoords.y;
        if (stats->minCoords.z < minCoords.z)
            minCoords.z = stats->minCoords.z;
    }

    center = (maxCoords + minCoords) * 0.5f;
    radius = math::length((maxCoords - minCoords) * 0.5f);

    this->mesh = mesh;
}

bool Sphere::is_on_frustrum(const Frustum &frustum) const

{
    const Vec3 globalScale = mesh->get_scale();

    const Vec3 globalCenter{mesh->get_model_matrix() * Vec4(center, 1.f)};

    const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);
    const float globalRadius = radius * maxScale;

    return (frustum.leftFace.get_signed_distance(globalCenter) >= -globalRadius &&
            frustum.rightFace.get_signed_distance(globalCenter) >= -globalRadius &&
            frustum.farFace.get_signed_distance(globalCenter) >= -globalRadius &&
            frustum.nearFace.get_signed_distance(globalCenter) >= -globalRadius &&
            frustum.topFace.get_signed_distance(globalCenter) >= -globalRadius &&
            frustum.bottomFace.get_signed_distance(globalCenter) >= -globalRadius);
}

Geometry *Mesh::change_geometry(Geometry *g, size_t id)
{
    if (m_geometry.size() < id + 1)
    {
        ERR_LOG("Not enough geometry slots");
        return nullptr;
    }
    Geometry *old_g = m_geometry[id];
    m_geometry[id] = g;
    return old_g;
}
IMaterial *Mesh::change_material(IMaterial *m, size_t id)
{
    if (m_material.size() < id + 1)
    {
        ERR_LOG("Not enough material slots");
        return nullptr;
    }

    IMaterial *old_m = m_material[id];
    m_material[id] = m;
    return old_m;
}

Mesh *Mesh::clone() const
{
    Mesh *mesh = new Mesh();
    mesh->m_material = m_material;
    mesh->m_geometry = m_geometry;
    mesh->setup_volume();
    mesh->set_name(m_name + std::string(" clone"));
    mesh->set_transform(m_transform);
    m_instanceCount++;
    return mesh;
}

void Mesh::set_volume_type(VolumeType t)
{
    m_volumeType = t;
    if (m_volume)
        delete m_volume;

    switch (m_volumeType)
    {
    case SPHERE_VOLUME:
        m_volume = new Sphere();
        break;
   
    case OBB_VOLUME:
        /* code */
        break;
    }
    m_volume->setup(this);
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END