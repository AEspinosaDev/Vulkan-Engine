#include <engine/core/scene/mesh.h>
#include <engine/utilities/loaders.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

int Mesh::m_instanceCount = 0;

void Sphere::setup(Mesh *const mesh)
{
    Vec3 maxCoords = {0.0f, 0.0f, 0.0f};
    Vec3 minCoords = {INFINITY, INFINITY, INFINITY};

    for (Geometry *g : mesh->get_geometries())
    {
        const  GeometricData* stats = g->get_geometric_data();

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
    radius = math::length( (maxCoords - minCoords) * 0.5f);

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
    mesh->setup_volume();
    mesh->set_name(m_name+std::string(" clone"));
    mesh->set_transform(m_transform);
    m_instanceCount++;
    return mesh;
}
Mesh *Mesh::create_quad()
{
    Mesh *quad = new Mesh();
    Geometry *g = new Geometry();
    
    g->fill(
        {{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
         {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
         {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
         {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}}},

        {0, 1, 2, 1, 3, 2});

    quad->set_geometry(g);
    quad->setup_volume();

    return quad;
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