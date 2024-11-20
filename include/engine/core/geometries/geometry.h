/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <engine/common.h>
#include <engine/graphics/accel.h>
#include <engine/graphics/vao.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class Geometry;

struct GeometricData {
    std::vector<uint32_t>         vertexIndex;
    std::vector<Graphics::Vertex> vertexData;
    std::vector<Graphics::Voxel>  voxelData;

    // Stats
    Vec3 maxCoords;
    Vec3 minCoords;
    Vec3 center;

    bool loaded{false};

    void compute_statistics();
};

/*
Class that defines the mesh geometry. Can be setup by filling it with a canonical vertex type array.
*/
class Geometry
{

  private:
    Graphics::VAO  m_VAO  = {};
    Graphics::BLAS m_BLAS = {};

    GeometricData m_properties = {};
    size_t        m_materialID = 0;

    friend Graphics::VertexArrays* const get_VAO(Geometry* g);
    friend Graphics::BLAS* const         get_BLAS(Geometry* g);

  public:
    Geometry() {
    }

    inline size_t get_material_ID() const {
        return m_materialID;
    }
    inline void set_material_ID(size_t id) {
        m_materialID = id;
    }

    inline bool data_loaded() const {
        return m_properties.loaded;
    }
    inline bool indexed() const {
        return !m_properties.vertexIndex.empty();
    }

    inline const GeometricData* get_properties() const {
        return &m_properties;
    }
    inline bool create_voxel_AS() const {
        return m_BLAS.topology == AccelGeometryType::AABBs;
    }
    inline void create_voxel_AS(bool op) {
        m_BLAS.topology = op ? AccelGeometryType::AABBs : AccelGeometryType::TRIANGLES;
    }
    ~Geometry() {
    }

    void             fill(std::vector<Graphics::Vertex> vertexInfo);
    void             fill(std::vector<Graphics::Vertex> vertexInfo, std::vector<uint32_t> vertexIndex);
    void             fill(Vec3* pos, Vec3* normal, Vec2* uv, Vec3* tangent, uint32_t vertNumber);
    void             fill_voxel_array(std::vector<Graphics::Voxel> voxels);
    static Geometry* create_quad();
    static Geometry* create_cube();
};

Graphics::VertexArrays* const get_VAO(Geometry* g);
Graphics::BLAS* const         get_BLAS(Geometry* g);

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END;

#endif // VK_GEOMETRY_H