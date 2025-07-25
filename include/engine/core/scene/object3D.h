/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <engine/common.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

struct Transform {

    Mat4 worldMatrix;
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    Vec3 right;
    Vec3 up;
    Vec3 forward;

public:
    Transform( Mat4 worldMatrix = Mat4( 1.0f ),
               Vec3 rotation    = Vec3( 0.0f ),
               Vec3 scale       = Vec3( 1.0f ),
               Vec3 position    = Vec3( 0.0f ),
               Vec3 up          = Vec3( 0.0f, 1.0f, 0.0f ),
               Vec3 forward     = Vec3( 0.0f, 0.0f, 1.0f ),
               Vec3 right       = Vec3( 1.0f, 0.0f, 0.0f )

                   )
        : position( position )
        , scale( scale )
        , rotation( rotation )
        , up( up )
        , forward( forward )
        , right( right )
        , worldMatrix( worldMatrix ) {
    }
};

class Object3D
{
protected:
    const ObjectType TYPE;
    std::string      m_name;

    Transform m_transform;

    std::vector<Object3D*> m_children;
    Object3D*              m_parent;

    bool enabled;
    bool m_isSelected { false };
    bool isDirty { true };

public:
    Object3D( const std::string na, ObjectType t )
        : TYPE( t )
        , m_name( na )
        , enabled( true )
        , m_parent( nullptr ) {
        m_transform.position = Vec3( 0.0f );
    }

    Object3D( const std::string na, Vec3 p, ObjectType t )
        : TYPE( t )
        , m_name( na )
        , enabled( true )
        , m_parent( nullptr ) {
        m_transform.position = p;
    }

    Object3D( Vec3 p, ObjectType t )
        : TYPE( t )
        , m_name( "" )
        , enabled( true )
        , m_parent( nullptr ) {
        m_transform.position = p;
    }
    Object3D( ObjectType t )
        : TYPE( t )
        , m_name( "" )
        , enabled( true )
        , m_parent( nullptr ) {
        m_transform.position = Vec3( 0.0f );
    }

    Object3D()
        : TYPE( ObjectType::OTHER )
        , m_name( "" )
        , enabled( true )
        , m_parent( nullptr ) {
        m_transform.position = Vec3( 0.0f );
    }

    ~Object3D() {
        for ( Object3D* obj : m_children )
        {
            delete obj;
        }
        m_children.clear();
    }

    virtual inline ObjectType get_type() const {
        return TYPE;
    };
    virtual void set_position( const Vec3 p ) {
        m_transform.position = p;
        isDirty              = true;
    }

    virtual inline Vec3 get_position() {
        return m_transform.position;
    };

    virtual void set_rotation( const Vec3 p, bool radians = false ) {
        if ( !radians )
            m_transform.rotation = glm::radians( p );
        else
            m_transform.rotation = p;

        // Update FORWARD
        Vec3 direction;
        direction.x         = cos( math::radians( p.x ) ) * cos( math::radians( p.y ) );
        direction.y         = sin( math::radians( p.y ) );
        direction.z         = sin( math::radians( p.x ) ) * cos( math::radians( p.y ) );
        m_transform.forward = -math::normalize( direction );
        // Update RIGHT
        m_transform.right = math::cross( m_transform.forward, Vec3( 0, 1, 0 ) );
        // Update UP
        m_transform.up = math::cross( m_transform.right, m_transform.forward );

        isDirty = true;
    }

    virtual inline Vec3 get_rotation( bool radians = false ) {
        return radians ? m_transform.rotation : math::degrees( m_transform.rotation );
    };

    virtual void set_scale( const Vec3 s ) {
        m_transform.scale = s;
        isDirty           = true;
    }

    virtual inline void look_at( Vec3 pos, Vec3 target, Vec3 up ) {
        m_transform.position = pos;
        m_transform.up       = up;
        m_transform.forward  = math::normalize( target - pos );
    }

    virtual void set_scale( const float s ) {
        m_transform.scale = Vec3( s );
        isDirty           = true;
    }

    virtual inline Vec3 get_scale() {
        return m_transform.scale;
    }

    virtual inline Transform get_transform() {
        return m_transform;
    }

    virtual inline void set_active( const bool s ) {
        enabled = s;
        isDirty = true;
        for ( Object3D* child : m_children )
            child->set_active( s );
    }

    virtual inline float get_pitch() {
        return m_transform.rotation.y;
    }

    virtual inline void set_pitch( float p ) {
        set_rotation( { m_transform.rotation.x, p, m_transform.rotation.z } );
    }

    virtual inline void set_yaw( float p ) {
        set_rotation( { p, m_transform.rotation.y, m_transform.rotation.z } );
    }

    virtual inline float get_yaw() {
        return m_transform.rotation.x;
    }

    virtual inline bool is_active() {
        return enabled;
    }

    virtual inline bool is_dirty() {
        return isDirty;
    }

    virtual inline std::string get_name() {
        return m_name;
    }

    virtual inline void set_name( std::string s ) {
        m_name = s;
    }

    virtual void set_transform( Transform t ) {
        m_transform = t;
        isDirty     = true;
    }

    virtual Mat4 get_model_matrix() {
        //  Dirty flag
        if ( isDirty )
        {

            m_transform.worldMatrix = Mat4( 1.0f );
            m_transform.worldMatrix = math::translate( m_transform.worldMatrix, m_transform.position );
            m_transform.worldMatrix = math::rotate( m_transform.worldMatrix, m_transform.rotation.x, Vec3( 1, 0, 0 ) );
            m_transform.worldMatrix = math::rotate( m_transform.worldMatrix, m_transform.rotation.y, Vec3( 0, 1, 0 ) );
            m_transform.worldMatrix = math::rotate( m_transform.worldMatrix, m_transform.rotation.z, Vec3( 0, 0, 1 ) );
            m_transform.worldMatrix = math::scale( m_transform.worldMatrix, m_transform.scale );

            isDirty = false;
        }

        return m_parent ? m_parent->get_model_matrix() * m_transform.worldMatrix : m_transform.worldMatrix;
    }

    virtual void add_child( Object3D* child ) {
        child->m_parent = this;
        m_children.push_back( child );
    }

    virtual std::vector<Object3D*> get_children() const {
        return m_children;
    }

    // virtual set_parent(Object3D* parent){m_parent=parent;}

    virtual void reconcile() {
        if ( isDirty )
            isDirty = false;
    };

    inline bool is_selected() const {
        return m_isSelected;
    }
    inline void set_selected( bool op ) {
        m_isSelected = op;
    }

    virtual Object3D* get_parent() const {
        return m_parent;
    }

    struct GPUPayload {
        Mat4 model;
        Vec4 otherParams1; // x is affected by fog, y is receive shadows, z cast shadows, w is line/point width
        Vec4 otherParams2; // x is selected // is affected by ambient light
    };
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
#endif