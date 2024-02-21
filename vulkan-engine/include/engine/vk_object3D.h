#ifndef VK_OBJECT_3D
#define VK_OBJECT_3D

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/vk_common.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct Transform
{

    Mat4 worldMatrix;
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    Vec3 right;
    Vec3 up;
    Vec3 forward;

public:
    Transform(
        Mat4 worldMatrix = Mat4(1.0f),
        Vec3 rotation = Vec3(0.0f),
        Vec3 scale = Vec3(1.0f),
        Vec3 position = Vec3(0.0f),
        Vec3 up = Vec3(0.0f, 1.0f, 0.0f),
        Vec3 forward = Vec3(0.0f, 0.0f, 1.0f),
        Vec3 right = Vec3(1.0f, 0.0f, 0.0f)

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
        m_transform.position = Vec3(0.0f);
    }

    Object3D(const std::string na, Vec3 p, ObjectType t) : TYPE(t), m_name(na), enabled(true),
                                                           m_parent(nullptr)
    {
        m_transform.position = p;
    }

    Object3D(Vec3 p, ObjectType t) : TYPE(t), m_name(""), enabled(true),
                                     m_parent(nullptr)
    {
        m_transform.position = p;
    }
    Object3D(ObjectType t) : TYPE(t), m_name(""), enabled(true),
                             m_parent(nullptr)
    {
        m_transform.position = Vec3(0.0f);
    }

    Object3D() : TYPE(OTHER), m_name(""), enabled(true),
                 m_parent(nullptr)
    {
        m_transform.position = Vec3(0.0f);
    }

    ~Object3D()
    {
        // delete[] children;
        delete m_parent;
    }

    virtual inline ObjectType get_type() const { return TYPE; };
    virtual void set_position(const Vec3 p)
    {
        m_transform.position = p;
        isDirty = true;
    }

    virtual inline Vec3 get_position() { return m_transform.position; };

    virtual void set_rotation(const Vec3 p, bool radians = false)
    {
        if (!radians)
            m_transform.rotation = glm::radians(p);
        else
            m_transform.rotation = p;

        // Update forward
        Vec3 direction;
        direction.x = cos(math::radians(p.x)) * cos(math::radians(p.y));
        direction.y = sin(math::radians(p.y));
        direction.z = sin(math::radians(p.x)) * cos(math::radians(p.y));
        m_transform.forward = -math::normalize(direction);
        // Update up

        // Update right
        m_transform.right = math::cross(m_transform.forward, m_transform.up);

        isDirty = true;
    }

    virtual inline Vec3 get_rotation() { return m_transform.rotation; };

    virtual void set_scale(const Vec3 s)
    {
        m_transform.scale = s;
        isDirty = true;
    }

    virtual void set_scale(const float s)
    {
        m_transform.scale = Vec3(s);
        isDirty = true;
    }

    virtual inline Vec3 get_scale() { return m_transform.scale; }

    virtual inline Transform get_transform() { return m_transform; }

    virtual inline void set_active(const bool s)
    {
        enabled = s;
        isDirty = true;
    }

    virtual inline float get_pitch() { return m_transform.rotation.y; }

    virtual inline void set_pitch(float p) { set_rotation({m_transform.rotation.x, p, m_transform.rotation.z}); }

    virtual inline void set_yaw(float p) { set_rotation({p, m_transform.rotation.y, m_transform.rotation.z}); }

    virtual inline float get_yaw() { return m_transform.rotation.x; }

    virtual inline bool is_active() { return enabled; }

    virtual inline bool is_dirty() { return isDirty; }

    virtual inline std::string get_name() { return m_name; }

    virtual inline void set_name(std::string s) { m_name = s; }

    virtual void set_transform(Transform t)
    {
        m_transform = t;
        isDirty = true;
    }

    virtual Mat4 get_model_matrix()
    {
        if (isDirty)
        {

            m_transform.worldMatrix = Mat4(1.0f);
            m_transform.worldMatrix = math::translate(m_transform.worldMatrix, m_transform.position);
            m_transform.worldMatrix = math::rotate(m_transform.worldMatrix, m_transform.rotation.x, Vec3(1, 0, 0));
            m_transform.worldMatrix = math::rotate(m_transform.worldMatrix, m_transform.rotation.y, Vec3(0, 1, 0));
            m_transform.worldMatrix = math::rotate(m_transform.worldMatrix, m_transform.rotation.z, Vec3(0, 0, 1));
            m_transform.worldMatrix = math::scale(m_transform.worldMatrix, m_transform.scale);

            isDirty = false;
        }
        //  Dirty flag
        m_transform.worldMatrix = m_parent ? m_parent->get_model_matrix() * m_transform.worldMatrix : m_transform.worldMatrix;
        return m_transform.worldMatrix;
    }

    virtual void add_child(Object3D *child)
    {
        child->m_parent = this;
        m_children.push_back(child);
    }

    virtual std::vector<Object3D *> get_children() const { return m_children; }

    // virtual set_parent(Object3D* parent){m_parent=parent;}

    virtual void reconcile()
    {
        if (isDirty)
            isDirty = false;
    };

    virtual Object3D *get_parent() const { return m_parent; }
};
VULKAN_ENGINE_NAMESPACE_END
#endif