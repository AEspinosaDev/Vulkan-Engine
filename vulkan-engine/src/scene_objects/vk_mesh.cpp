#include <engine/scene_objects/vk_mesh.h>
#include <engine/utilities/vk_loaders.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

int Mesh::m_instanceCount = 0;

void Sphere::setup(Mesh *const mesh)
{
    Vec3 maxCoords = {0.0f, 0.0f, 0.0f};
    Vec3 minCoords = {INFINITY, INFINITY, INFINITY};

    for (Geometry *g : mesh->get_geometries())
    {
        GeometryStats stats;
        stats.compute_statistics(g);

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
    }

    center = (maxCoords - minCoords) * 0.5f;
    radius = math::length(center);
    // center=Vec3(0.0f);
    // radius=1.0f;

    this->mesh = mesh;

    // std::cout << center.x << center.y << center.z << std::endl;
}

bool Sphere::is_on_frustrum(const Frustum &frustum) const

{
    const Vec3 globalScale = mesh->get_scale();

    const Vec3 globalCenter{mesh->get_model_matrix() * Vec4(center, 1.f)};

    const float maxScale = max(max(globalScale.x, globalScale.y), globalScale.z);
    Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

    return (frustum.leftFace.get_signed_distance(center) > -radius &&
            frustum.rightFace.get_signed_distance(center) > -radius &&
            frustum.farFace.get_signed_distance(center) > -radius &&
            frustum.nearFace.get_signed_distance(center) > -radius &&
            frustum.topFace.get_signed_distance(center) > -radius &&
            frustum.bottomFace.get_signed_distance(center) > -radius);
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
Material *Mesh::change_material(Material *m, size_t id)
{
    if (m_material.size() < id + 1)
    {
        ERR_LOG("Not enough material slots");
        return nullptr;
    }

    Material *old_m = m_material[id];
    m_material[id] = m;
    return old_m;
}

void Mesh::load_file(const std::string fileName, bool asyncCall, bool overrideGeometry)
{
    m_fileRoute = fileName;
    loaders::load_3D_file(this, fileName, asyncCall, overrideGeometry);
}
Mesh *Mesh::clone() const
{
    Mesh *mesh = new Mesh();
    for (auto m : m_material)
    {
        mesh->set_material(m);
    }
    for (auto g : m_geometry)
    {
        mesh->set_geometry(g);
    }
    return mesh;
}
void Mesh::set_volume_type(VolumeType t)
{
    m_volumeType = t;
    if (m_volume)
        delete m_volume;

    switch (m_volumeType)
    {
    case SPHERE:
        m_volume = new Sphere();
        break;
    case AABB:
        /* code */
        break;
    case OBB:
        /* code */
        break;
    }
    m_volume->setup(this);
}

VULKAN_ENGINE_NAMESPACE_END