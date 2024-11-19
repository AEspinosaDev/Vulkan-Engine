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
struct Volume {
    Mesh* mesh;
    Vec3  center{0.0f, 0.0f, 0.0f};

    virtual void setup(Mesh* const mesh) = 0;

    virtual bool is_on_frustrum(const Frustum& frustum) const = 0;
};
struct Sphere : public Volume {
    float radius{0.0f};

    Sphere() = default;

    Sphere(const Vec3 c, const float r)
        : radius(r) {
        center = c;
    }

    virtual void setup(Mesh* const mesh);

    virtual bool is_on_frustrum(const Frustum& frustum) const;
};
struct AABB : public Volume {
    // TO DO
};

/*
Class used to represent a 3D model instance.
*/
class Mesh : public Object3D
{
  protected:
    std::vector<Geometry*>  m_geometry;
    std::vector<IMaterial*> m_material;

    VolumeType  m_volumeType     = SPHERE_VOLUME;
    Volume*     m_volume         = nullptr;
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
        , m_volume(new Sphere()) {
        Mesh::m_instanceCount++;
    }
    Mesh(Geometry* geom, IMaterial* mat)
        : Object3D("Mesh #" + std::to_string(Mesh::m_instanceCount), ObjectType::MESH)
        , m_volume(new Sphere()) {
        Mesh::m_instanceCount++;
        m_geometry.push_back(geom);
        m_material.push_back(mat);
    }
    ~Mesh() {
        // delete m_geometry;
    }
    /**
     * Returns the geometry in the slot.
     */
    inline Geometry* const get_geometry(size_t id = 0) const {
        return m_geometry.size() >= id + 1 ? m_geometry[id] : nullptr;
    }
    inline std::vector<Geometry*> get_geometries() const {
        return m_geometry;
    };
    /**
     * Change the geometry in the given slot and returns the old one ref.
     */
    Geometry* change_geometry(Geometry* g, size_t id = 0);
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
    IMaterial* change_material(IMaterial* m, size_t id = 0);
    /*
     * Adds this geometry in the next free slot. It is important to set correctly the id of the material slot this
     * geometry is pointing.
     */
    inline void push_geometry(Geometry* g) {
        m_geometry.push_back(g);
        setup_volume();
    }
    /*
     * Adds this material in the next free slot
     */
    inline void push_material(IMaterial* m) {
        m_material.push_back(m);
    }

    inline size_t get_num_geometries() const {
        return m_geometry.size();
    }
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
    inline VolumeType get_volume_type() const {
        return m_volumeType;
    }
    void set_volume_type(VolumeType t);

    inline void setup_volume() {
        if (m_volume)
            m_volume->setup(this);
    }

    inline const Volume* const get_bounding_volume() const {
        return m_volume;
    }

    inline std::string get_file_route() const {
        return m_fileRoute;
    }

    inline void set_file_route(std::string r) {
        m_fileRoute = r;
    }

    Mesh* clone() const;
};
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif // VK_MESH_H