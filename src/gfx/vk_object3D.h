#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkeng
{

    struct Transform
    {

        glm::mat4 worldMatrix;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
        glm::vec3 right;
        glm::vec3 up;
        glm::vec3 forward;

    public:
        Transform(
            glm::mat4 worldMatrix = glm::mat4(1.0f),
            glm::vec3 rotation = glm::vec3(0.0f),
            glm::vec3 scale = glm::vec3(1.0f),
            glm::vec3 position = glm::vec3(0.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f)

                ) : position(position),
                    scale(scale),
                    rotation(rotation),
                    up(up),
                    forward(forward),
                    right(right),
                    worldMatrix(worldMatrix)
        {
        }
    };

    enum ObjectType
    {
        MESH = 0,
        LIGHT = 1,
        OTHER = 2
    };

    class Object3D
    {
    protected:
        const ObjectType TYPE;
        std::string m_name;
        Transform m_transform;
        std::vector<Object3D *> m_children;
        Object3D *m_parent;

        bool enabled;
        bool isDirty{true};

    public:
        Object3D(const std::string na, ObjectType t) : TYPE(t), m_name(na), enabled(true),
                                                       m_parent(nullptr)
        {
            m_transform.position = glm::vec3(0.0f);
        }

        Object3D(const std::string na, glm::vec3 p, ObjectType t) : TYPE(t), m_name(na), enabled(true),
                                                                    m_parent(nullptr)
        {
            m_transform.position = p;
        }

        Object3D(glm::vec3 p, ObjectType t) : TYPE(t), m_name(""), enabled(true),
                                              m_parent(nullptr)
        {
            m_transform.position = p;
        }
        Object3D(ObjectType t) : TYPE(t), m_name(""), enabled(true),
                     m_parent(nullptr)
        {
            m_transform.position = glm::vec3(0.0f);
        }

        Object3D() : TYPE(OTHER), m_name(""), enabled(true),
                     m_parent(nullptr)
        {
            m_transform.position = glm::vec3(0.0f);
        }

        ~Object3D()
        {
            // delete[] children;
            delete m_parent;
        }

        virtual inline ObjectType get_type() const {return TYPE;};
        virtual void set_position(const glm::vec3 p)
        {
            m_transform.position = p;
            isDirty = true;
        }

        virtual inline glm::vec3 get_position() { return m_transform.position; };

        virtual void set_rotation(const glm::vec3 p)
        {
            m_transform.rotation = p;
            isDirty = true;
        }

        virtual inline glm::vec3 get_rotation() { return m_transform.rotation; };

        virtual void set_scale(const glm::vec3 s)
        {
            m_transform.scale = s;
            isDirty = true;
        }

        virtual inline glm::vec3 get_scale() { return m_transform.scale; }

        virtual inline Transform get_transform() { return m_transform; }

        virtual inline void set_active(const bool s)
        {
            enabled = s;
            isDirty = true;
        }

        virtual inline bool is_active() { return enabled; }

        virtual inline bool is_dirty() { return isDirty; }

        virtual inline std::string get_name() { return m_name; }

        virtual inline void set_name(std::string s) { m_name = s; }

        virtual void set_transform(Transform t)
        {
            m_transform = t;
            isDirty = true;
        }

        virtual glm::mat4 get_model_matrix()
        {
            if (isDirty)
            {

                m_transform.worldMatrix = glm::mat4(1.0f);
                m_transform.worldMatrix = glm::translate(m_transform.worldMatrix, m_transform.position);
                m_transform.worldMatrix = glm::rotate(m_transform.worldMatrix, m_transform.rotation.x, glm::vec3(1, 0, 0));
                m_transform.worldMatrix = glm::rotate(m_transform.worldMatrix, m_transform.rotation.y, glm::vec3(0, 1, 0));
                m_transform.worldMatrix = glm::rotate(m_transform.worldMatrix, m_transform.rotation.z, glm::vec3(0, 0, 1));
                m_transform.worldMatrix = glm::scale(m_transform.worldMatrix, m_transform.scale);
                // iterate though parents for multypling model matrix
                //   and set children to dirty  for them to do the same!!!!!!!!
                //  .............
                //  Dirty flag

                isDirty = false;
            }
            return m_transform.worldMatrix;
        }

        virtual void add_child(Object3D *child)
        {
            child->m_parent = this;
            m_children.push_back(child);
        }

        virtual std::vector<Object3D *> get_children() const { return m_children; }

        // virtual set_parent(Object3D* parent){m_parent=parent;}

        virtual Object3D *get_parent() const { return m_parent; }
    };
}