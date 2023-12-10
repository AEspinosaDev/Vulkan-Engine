#ifndef VK_MESH_H
#define VK_MESH_H

#include "../vk_object3D.h"
#include "../vk_geometry.h"
#include "../vk_material.h"

namespace vke
{

	class Mesh : public Object3D
	{
		Geometry *m_geometry;
		Material *m_material;

		bool m_affectedByFog{true};
		bool m_castShadows{true};
		bool m_receiveShadows{true};

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
			if (m_geometry != nullptr)
				delete m_geometry;
			m_geometry = g;
		}
		Material *get_material() const { return m_material; }
		void set_material(Material *m)
		{
			if (m_material != nullptr)
				delete m_material;
			m_material = m;
		}
		inline void set_cast_shadows(bool op) { m_castShadows = op; }
		inline bool get_cast_shadows() { return m_castShadows; }
		inline void set_receive_shadows(bool op) { m_receiveShadows = op; }
		inline bool get_recive_shadows() { return m_castShadows; }
		inline void set_affected_by_fog(bool op) { m_affectedByFog = op; }
		inline bool is_affected_by_fog() { return m_affectedByFog; }

		void load_file(const std::string fileName);
	};

}
#endif // VK_MESH_H