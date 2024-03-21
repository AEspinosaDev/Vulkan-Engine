/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VK_MESH
#define VK_MESH

#include <engine/vk_object3D.h>
#include <engine/vk_geometry.h>
#include <engine/vk_material.h>
#include <engine/scene_objects/vk_camera.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

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

	Sphere(const Vec3 c, const float r) : center(c), radius(r) {}

	virtual void setup(Mesh *const mesh);

	virtual bool is_on_frustrum(const Frustum &frustum) const;
};
struct AABB : public Volume
{
	// TO DO
};

/*
Class used to represent a 3D model.
*/
class Mesh : public Object3D
{
	std::vector<Geometry *> m_geometry;
	std::vector<Material *> m_material;

	VolumeType m_volumeType{SPHERE};
	Volume *m_volume{nullptr};

	bool m_affectedByFog{true};
	bool m_castShadows{true};
	bool m_receiveShadows{true};

	std::string m_fileRoute{"None"};

	static Material *m_debugMaterial;
	static int m_instanceCount;

public:
	Mesh() : Object3D("Mesh #" + std::to_string(Mesh::m_instanceCount), MESH), m_volume(new Sphere())
	{
		Mesh::m_instanceCount++;
	}
	Mesh(Geometry *geom, Material *mat) : Object3D("Mesh #" + std::to_string(Mesh::m_instanceCount), MESH), m_volume(new Sphere())
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
	inline Geometry *get_geometry(size_t id = 0) const { return m_geometry.size() >= id + 1 ? m_geometry[id] : nullptr; }
	inline std::vector<Geometry *> get_geometries() const { return m_geometry; };
	/**
	 * Change the geometry in the given slot and returns the old one ref.
	 */
	Geometry *change_geometry(Geometry *g, size_t id = 0);
	/**
	 * Returns the material in the slot.
	 */
	inline Material *get_material(size_t id = 0) const { return m_material.size() >= id + 1 ? m_material[id] : nullptr; }
	inline std::vector<Material *> get_materials() const { return m_material; };
	/**
	 * Change the material in the given slot and returns the old one ref.
	 */
	Material *change_material(Material *m, size_t id = 0);
	/*
	 * Adds this geometry in the next free slot. It is important to set correctly the id of the material slot this geometry is pointing.
	 */
	inline void set_geometry(Geometry *g)
	{
		m_geometry.push_back(g);
	}
	/*
	 * Adds this material in the next free slot
	 */
	inline void set_material(Material *m)
	{
		m_material.push_back(m);
	}
	inline size_t get_num_geometries() const { return m_geometry.size(); }
	inline size_t get_num_materials() const { return m_material.size(); }

	inline void set_cast_shadows(bool op) { m_castShadows = op; }
	inline bool get_cast_shadows() const { return m_castShadows; }
	inline void set_receive_shadows(bool op) { m_receiveShadows = op; }
	inline Material *set_debug_material(size_t id = 0)
	{
		Material *m = get_material(id);
		m_material[id] = m_debugMaterial;
		return m;
	}
	inline bool get_recive_shadows() const { return m_castShadows; }
	inline void set_affected_by_fog(bool op) { m_affectedByFog = op; }
	inline bool is_affected_by_fog() const { return m_affectedByFog; }

	inline VolumeType get_volume_type() const { return m_volumeType; }
	void set_volume_type(VolumeType t);

	inline void setup_volume()
	{
		if (m_volume)
			m_volume->setup(this);
	}

	inline const Volume *const get_bounding_volume() const { return m_volume; }

	/*
	 * Asynchornously loads any kind of supported mesh file (ply, obj). Async can be deactivated.
	 */
	void load_file(const std::string fileName, bool asyncCall = true, bool overrideGeometry = false);

	inline std::string get_file_route() const { return m_fileRoute; }

	Mesh *clone() const;
};

VULKAN_ENGINE_NAMESPACE_END

#endif // VK_MESH_H