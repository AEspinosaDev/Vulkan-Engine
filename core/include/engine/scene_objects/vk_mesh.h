#ifndef VK_MESH_H
#define VK_MESH_H

#include "../vk_object3D.h"
#include "../vk_geometry.h"
#include "../vk_material.h"

namespace vke
{
	/*
	Class containing data to represent a 3D model.
	*/
	class Mesh : public Object3D
	{
		Geometry *m_geometry;
		Material *m_material;

		bool m_affectedByFog{true};
		bool m_castShadows{true};
		bool m_receiveShadows{true};

		// TO DO: Initialize this material and call it in the renderer if mesh doesnt have a material
		static Material *m_debugMaterial;

	public:
		Mesh() : Object3D(MESH), m_geometry(nullptr), m_material(nullptr) {}
		Mesh(Geometry *geom, Material *mat) : Object3D(MESH), m_geometry(geom), m_material(mat) {}
		~Mesh()
		{
			delete m_geometry;
		}
		Geometry *get_geometry() const { return m_geometry; }
		void set_geometry(Geometry *g)
		{
			m_geometry = g;
		}
		Material *get_material() const { return m_material; }
		void set_material(Material *m)
		{
			m_material = m;
		}
		inline void set_cast_shadows(bool op) { m_castShadows = op; }
		inline bool get_cast_shadows() const { return m_castShadows; }
		inline void set_receive_shadows(bool op) { m_receiveShadows = op; }
		inline Material *set_debug_material()
		{
			Material *m = m_material;
			m_material = m_debugMaterial;
			return m;
		}
		inline bool get_recive_shadows() const { return m_castShadows; }
		inline void set_affected_by_fog(bool op) { m_affectedByFog = op; }
		inline bool is_affected_by_fog() const { return m_affectedByFog; }

		bool load_file(const std::string fileName, bool overrideGeometry = false);

		Mesh *clone() const;
	};

}
#endif // VK_MESH_H