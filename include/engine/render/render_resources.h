/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDER_RESOURCES
#define RENDER_RESOURCES
#include <engine/graphics/device.h>

#include <engine/core/geometries/geometry.h>
#include <engine/core/scene/scene.h>
#include <engine/core/textures/texture_template.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

enum class ResourceType : uint8_t
{
    Buffer,
    Texture,
    VAO,
    Accel
};

struct ResourceHandle {
    ResourceType type;
    uint32_t     id = 0;

    bool operator==( const ResourceHandle& other ) const {
        return type == other.type && id == other.id;
    }
};

class ResourcePool
{
protected:
    std::shared_ptr<Graphics::Device> m_device;

    std::unordered_map<uint32_t, Graphics::Buffer>  m_buffers;
    std::unordered_map<uint32_t, Graphics::Texture> m_textures;
    std::unordered_map<uint32_t, Graphics::VAO>     m_vaos;
    std::unordered_map<uint32_t, Graphics::Accel>   m_accels;

    uint32_t m_nextId = 1; // 0 is reserved for "invalid"

    std::unordered_map<std::string, ResourceHandle> m_nameToHandle;

    template <typename MapT, typename ResourceT>
    ResourceHandle store_resource( MapT& map, ResourceType type, ResourceT&& resource ) {
        uint32_t id = m_nextId++;
        map[id]     = std::move( resource );
        return ResourceHandle { type, id };
    }

public:
    void init( std::shared_ptr<Graphics::Device> device ) {
        m_device = std::move( device );
    }
    

    // Register and return handles
    template <typename... UBOs>
    ResourceHandle register_ubo() {
        size_t           totalSize = ( m_device->pad_uniform_buffer_size( sizeof( UBOs ) ) + ... );
        Graphics::Buffer buffer    = m_device->create_buffer_VMA(
            totalSize,
            BUFFER_USAGE_UNIFORM_BUFFER,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            uint32_t( totalSize ) );
        return store_resource( m_buffers, ResourceType::Buffer, std::move( buffer ) );
    }

    ResourceHandle register_texture( Graphics::Texture texture ) {
        return store_resource( m_textures, ResourceType::Texture, std::move( texture ) );
    }

    ResourceHandle register_vao( Graphics::VAO vao ) {
        return store_resource( m_vaos, ResourceType::VAO, std::move( vao ) );
    }

    ResourceHandle register_accel( Graphics::Accel accel ) {
        return store_resource( m_accels, ResourceType::Accel, std::move( accel ) );
    }

    // Accessors
    const Graphics::Buffer& get_buffer( ResourceHandle h ) const {
        assert( h.type == ResourceType::Buffer );
        return m_buffers.at( h.id );
    }

    const Graphics::Texture& get_texture( ResourceHandle h ) const {
        assert( h.type == ResourceType::Texture );
        return m_textures.at( h.id );
    }

    const Graphics::VAO& get_vao( ResourceHandle h ) const {
        assert( h.type == ResourceType::VAO );
        return m_vaos.at( h.id );
    }

    const Graphics::Accel& get_accel( ResourceHandle h ) const {
        assert( h.type == ResourceType::Accel );
        return m_accels.at( h.id );
    }
    

    void set_name( ResourceHandle h, const std::string& name ) {
        m_nameToHandle[name] = h;
    }

    ResourceHandle get_handle( const std::string& name ) const {
        return m_nameToHandle.at( name );
    }
};
/*
Global Render Resources Data and Utilities
*/
struct Resources {
    Core::Geometry*              vignette;
    std::vector<Core::ITexture*> sharedTextures;
    Core::ITexture*              fallbackTexture2D;
    Core::ITexture*              fallbackCubeMap;
    Mat4                         prevViewProj;

    void init_shared_resources( const ptr<Graphics::Device>& device );
    void clean_shared_resources();

    static void upload_texture_data( const ptr<Graphics::Device>& device, Core::ITexture* const t );
    static void destroy_texture_data( Core::ITexture* const t );
    static void upload_geometry_data( const ptr<Graphics::Device>& device, Core::Geometry* const g, bool createAccelStructure = true );
    static void destroy_geometry_data( Core::Geometry* const g );
    static void upload_skybox_data( const ptr<Graphics::Device>& device, Core::Skybox* const sky );
    static void clean_scene( Core::Scene* const scene );
};

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END

// Hash support for ResourceHandle
namespace std {
template <>
struct hash<VKFW::Render::ResourceHandle> {
    size_t operator()( const VKFW::Render::ResourceHandle& h ) const {
        return hash<uint64_t>()( ( uint64_t( h.type ) << 32 ) | h.id );
    }
};
} // namespace std
#endif