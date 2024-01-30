#ifndef VK_MESH
#define VK_MESH

#include "../vk_object3D.h"
#include "../vk_geometry.h"
#include "../vk_material.h"

namespace vke
{
	/*
	Class used to represent a 3D model.
	*/
	class Mesh : public Object3D
	{
		std::vector<Geometry *> m_geometry;
		std::vector<Material *> m_material;

		bool m_affectedByFog{true};
		bool m_castShadows{true};
		bool m_receiveShadows{true};

		std::string m_fileRoute{"None"};

		static Material *m_debugMaterial;
		static int m_meshCount;

	public:
		Mesh() : Object3D("Mesh #" + std::to_string(Mesh::m_meshCount), MESH) { Mesh::m_meshCount++; }
		Mesh(Geometry *geom, Material *mat) : Object3D("Mesh #" + std::to_string(Mesh::m_meshCount), MESH)
		{
			m_geometry.push_back(geom);
			m_material.push_back(mat);

			Mesh::m_meshCount++;
		}
		~Mesh()
		{
			// delete m_geometry;
		}
		/**
		 * Returns the geometry in the slot.
		 */
		inline Geometry *get_geometry(size_t id = 0) const { return m_geometry.size() >= id + 1 ? m_geometry[id] : nullptr; }
		/**
		 * Change the geometry in the given slot and returns the old one ref.
		 */
		Geometry *change_geometry(Geometry *g, size_t id = 0);
		/**
		 * Returns the material in the slot.
		 */
		inline Material *get_material(size_t id = 0) const { return m_material.size() >= id + 1 ? m_material[id] : nullptr; }
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

		bool load_file(const std::string fileName, bool overrideGeometry = false);

		inline std::string get_file_route() const { return m_fileRoute; }

		Mesh *clone() const;
	};

}
#endif // VK_MESH_H