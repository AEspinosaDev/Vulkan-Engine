/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef MESH_H
#define MESH_H

#include <engine/core/geometries/geometry.h>
#include <engine/core/materials/material.h>
#include <engine/core/scene/camera.h>
#include <engine/core/scene/object3D.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

// Ahead declare
class Mesh;

/*
Bounding volume base struct
*/
struct BoundingVolume {
    const VolumeType TYPE;
    Object3D*        obj       = nullptr;
    Vec3             center    = {0.0f, 0.0f, 0.0f};
    Vec3             maxCoords = {0.0f, 0.0f, 0.0f};
    Vec3             minCoords = {INFINITY, INFINITY, INFINITY};

    BoundingVolume(VolumeType t, Object3D* o)
        : TYPE(t)
        , obj(o) {
    }
    ~BoundingVolume() {
    }

    virtual void setup(Mesh* const mesh) = 0;

    virtual bool is_on_frustrum(const Frustum& frustum) const = 0;
};
typedef BoundingVolume BVolume;
typedef BoundingVolume BV;

struct BoundingSphere : public BVolume {
    float radius{0.0f};

    BoundingSphere(Object3D* o)
        : BVolume(VolumeType::SPHERE_VOLUME, o) {
    }

    BoundingSphere(const Vec3 c, const float r, Object3D* o)
        : radius(r)
        , BVolume(VolumeType::SPHERE_VOLUME, o) {
        center = c;
    }

    virtual void setup(Mesh* const mesh);

    virtual bool is_on_frustrum(const Frustum& frustum) const;
};
struct AABB : public BVolume {

    AABB(Object3D* o)
        : BVolume(VolumeType::AABB_VOLUME, o) {
    }

    AABB(const Vec3 min, const Vec3 max, Object3D* o)
        : BoundingVolume(VolumeType::AABB_VOLUME, o) {
        minCoords = min;
        maxCoords = max;
        center    = (maxCoords + minCoords) * 0.5f;
    }

    virtual void setup(Mesh* const mesh);

    virtual bool is_on_frustrum(const Frustum& frustum) const;
};

/*
Class used to represent a 3D model instance.
*/
class Mesh : public Object3D
{
  protected:
    Geometry*               m_geometry;
    std::vector<IMaterial*> m_material;

    BV*         m_volume         = nullptr;
    bool        m_affectedByFog  = true;
    bool        m_castShadows    = true;
    bool        m_receiveShadows = true;
    bool        m_rayHittable    = true;
    std::string m_fileRoute      = "None";

    static IMaterial* m_debugMaterial;
    static int        m_instanceCount;

    Mesh(std::string name, ObjectType type)
        : Object3D(name, type) {
    }

  public:
    Mesh()
        : Object3D("Mesh #" + std::to_string(Mesh::m_instanceCount), ObjectType::MESH)
        , m_volume(nullptr) {
        Mesh::m_instanceCount++;
    }
    Mesh(Geometry* geom, IMaterial* mat)
        : Object3D("Mesh #" + std::to_string(Mesh::m_instanceCount), ObjectType::MESH)
        , m_volume(nullptr) {
        Mesh::m_instanceCount++;
        m_geometry = geom;
        m_material.push_back(mat);
    }
    ~Mesh() {
        // delete m_geometry;
    }

    inline Geometry* const get_geometry() const {
        return m_geometry;
    }
    inline void set_geometry(Geometry* g) {
        m_geometry = g;
        setup_volume();
    }
    /*
     * Adds this material in the next free slot
     */
    inline void add_material(IMaterial* m) {
        m_material.push_back(m);
    }
    /**
     * Returns the material in the slot.
     */
    inline IMaterial* get_material(size_t id = 0) const {
        return m_material.size() >= id + 1 ? m_material[id] : nullptr;
    }
    inline std::vector<IMaterial*> get_materials() const {
        return m_material;
    };
    /**
     * Change the material in the given slot and returns the old one ref.
     */
    IMaterial*    change_material(IMaterial* m, size_t id = 0);
    inline size_t get_num_materials() const {
        return m_material.size();
    }
    inline void cast_shadows(bool op) {
        m_castShadows = op;
    }
    inline bool cast_shadows() const {
        return m_castShadows;
    }
    inline void receive_shadows(bool op) {
        m_receiveShadows = op;
    }
    inline IMaterial* set_debug_material(size_t id = 0) {
        IMaterial* m   = get_material(id);
        m_material[id] = m_debugMaterial;
        return m;
    }
    inline bool receive_shadows() const {
        return m_castShadows;
    }
    inline void affected_by_fog(bool op) {
        m_affectedByFog = op;
    }
    inline bool affected_by_fog() const {
        return m_affectedByFog;
    }
    /*
    Control if the mesh takes part in the raytracing pipeline as a hittable target.
    */
    inline void ray_hittable(bool op) {
        m_rayHittable = op;
    }
    /*
    Says if the mesh takes part in the raytracing pipeline as a hittable target.
    */
    inline bool ray_hittable() const {
        return m_rayHittable;
    }

    inline const BV* const get_bounding_volume() const {
        return m_volume;
    }

    inline std::string get_file_route() const {
        return m_fileRoute;
    }

    inline void set_file_route(std::string r) {
        m_fileRoute = r;
    }

    void  setup_volume(VolumeType type = VolumeType::SPHERE_VOLUME);
    Mesh* clone() const;
};
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif // VK_MESH_H