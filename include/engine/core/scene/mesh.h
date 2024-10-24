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

namespace Core
{

// Ahead declare
class Mesh;

/*
Bounding volume base struct
*/
struct Volume
{
    Mesh *mesh;

    virtual void setup(Mesh *const mesh) = 0;

    virtual bool is_on_frustrum(const Frustum &frustum) const = 0;
};
struct Sphere : public Volume
{
    Vec3 center{0.0f, 0.0f, 0.0f};
    float radius{0.0f};

    Sphere() = default;

    Sphere(const Vec3 c, const float r) : center(c), radius(r)
    {
    }

    virtual void setup(Mesh *const mesh);

    virtual bool is_on_frustrum(const Frustum &frustum) const;
};
struct AABB : public Volume
{
    // TO DO
};

/*
Class used to represent a 3D model instance.
*/
class Mesh : public Object3D
{
    std::vector<Geometry *> m_geometry;
    std::vector<IMaterial *> m_material;

    VolumeType m_volumeType{SPHERE};
    Volume *m_volume{nullptr};

    bool m_affectedByFog{true};
    bool m_castShadows{true};
    bool m_receiveShadows{true};

    std::string m_fileRoute{"None"};

    static IMaterial *m_debugMaterial;
    static int m_instanceCount;

  public:
    Mesh() : Object3D("Mesh #" + std::to_string(Mesh::m_instanceCount), MESH), m_volume(new Sphere())
    {
        Mesh::m_instanceCount++;
    }
    Mesh(Geometry *geom, IMaterial *mat)
        : Object3D("Mesh #" + std::to_string(Mesh::m_instanceCount), MESH), m_volume(new Sphere())
    {
        Mesh::m_instanceCount++;
        m_geometry.push_back(geom);
        m_material.push_back(mat);
    }
    ~Mesh()
    {
        // delete m_geometry;
    }
    /**
     * Returns the geometry in the slot.
     */
    inline Geometry *const get_geometry(size_t id = 0) const
    {
        return m_geometry.size() >= id + 1 ? m_geometry[id] : nullptr;
    }
    inline std::vector<Geometry *> get_geometries() const
    {
        return m_geometry;
    };
    /**
     * Change the geometry in the given slot and returns the old one ref.
     */
    Geometry *change_geometry(Geometry *g, size_t id = 0);
    /**
     * Returns the material in the slot.
     */
    inline IMaterial *get_material(size_t id = 0) const
    {
        return m_material.size() >= id + 1 ? m_material[id] : nullptr;
    }
    inline std::vector<IMaterial *> get_materials() const
    {
        return m_material;
    };
    /**
     * Change the material in the given slot and returns the old one ref.
     */
    IMaterial *change_material(IMaterial *m, size_t id = 0);
    /*
     * Adds this geometry in the next free slot. It is important to set correctly the id of the material slot this
     * geometry is pointing.
     */
    inline void push_geometry(Geometry *g)
    {
        m_geometry.push_back(g);
        setup_volume();
    }
    /*
     * Adds this material in the next free slot
     */
    inline void push_material(IMaterial *m)
    {
        m_material.push_back(m);
    }

    inline size_t get_num_geometries() const
    {
        return m_geometry.size();
    }
    inline size_t get_num_materials() const
    {
        return m_material.size();
    }

    inline void set_cast_shadows(bool op)
    {
        m_castShadows = op;
    }
    inline bool get_cast_shadows() const
    {
        return m_castShadows;
    }
    inline void set_receive_shadows(bool op)
    {
        m_receiveShadows = op;
    }
    inline IMaterial *set_debug_material(size_t id = 0)
    {
        IMaterial *m = get_material(id);
        m_material[id] = m_debugMaterial;
        return m;
    }
    inline bool get_recive_shadows() const
    {
        return m_castShadows;
    }
    inline void set_affected_by_fog(bool op)
    {
        m_affectedByFog = op;
    }
    inline bool is_affected_by_fog() const
    {
        return m_affectedByFog;
    }

    inline VolumeType get_volume_type() const
    {
        return m_volumeType;
    }
    void set_volume_type(VolumeType t);

    inline void setup_volume()
    {
        if (m_volume)
            m_volume->setup(this);
    }

    inline const Volume *const get_bounding_volume() const
    {
        return m_volume;
    }

    inline std::string get_file_route() const
    {
        return m_fileRoute;
    }

    inline void set_file_route(std::string r)
    {
        m_fileRoute = r;
    }

    Mesh *clone() const;

    // void get_uniform_data(void *&data, size_t &size)
    // {
    //     struct ObjectUniforms
    //     {
    //         Mat4 model;
    //         Vec4 otherParams1; // x is affected by fog, y is receive shadows, z cast shadows
    //         Vec4 otherParams2; // x is selected
    //     };
    //     ObjectUniforms objectData;
    //     objectData.model = get_model_matrix();
    //     objectData.otherParams1 = {is_affected_by_fog(), get_recive_shadows(), get_cast_shadows(), false};
    //     objectData.otherParams2 = {is_selected(), 0.0, 0.0, 0.0};

    //     size = sizeof(ObjectUniforms); // Get size of the struct
    //     data = malloc(size);           // Allocate memory for the data

    //     if (data != nullptr)
    //     {
    //         std::memcpy(data, &objectData, size); // Copy the struct data into the allocated memory
    //     }
    // }
};
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif // VK_MESH_H