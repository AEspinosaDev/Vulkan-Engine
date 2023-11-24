#ifndef VK_MESH_H
#define VK_MESH_H

#include "vk_object3D.h"
#include "vk_geometry.h"

namespace vke
{

	class Mesh : public Object3D
	{
		Geometry *m_geometry;
		// Material* m_material;

	public:
		Mesh() : Object3D(MESH), m_geometry(nullptr) {}
		Mesh(Geometry *geom) : Object3D(MESH), m_geometry(geom) {}
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
		void load_file(const std::string fileName);
	};

}
#endif // VK_MESH_H