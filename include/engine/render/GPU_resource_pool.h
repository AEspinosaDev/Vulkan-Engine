/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#pragma once

#include <engine/core/geometries/geometry.h>
#include <engine/core/textures/texture.h>
#include <engine/core/textures/texture_template.h>
#include <engine/graphics/device.h>
#include <engine/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

class GPUResourcePool
{
    // Device PTR
    std::shared_ptr<Graphics::Device> m_device = nullptr;

    // Basic Reources
    Graphics::VAO   m_vignetteVAO;
    Graphics::Image m_fallbackImage2D;
    Graphics::Image m_fallbackImage3D;
    Graphics::Image m_fallbackCubemap;

    // Resources (GPU/CPU)
    std::unordered_map<std::string, Graphics::Buffer>
                                                     m_ubos;   // string resoruce name + actual buffer
    std::unordered_map<std::string, Graphics::Image> m_images; // string resoruce name + actual image

    template <typename UBO>
    size_t pad_size() const {
        return m_device->pad_uniform_buffer_size( sizeof( UBO ) );
    }

public:
    void init( const std::shared_ptr<Graphics::Device>& device );
    void cleanup();

    const Graphics::VAO& get_vignette_VAO() const {
        return m_vignetteVAO;
    }
    const Graphics::Image& get_fallback_image_2D() const {
        return m_fallbackImage2D;
    }
    const Graphics::Image& get_fallback_image_3D() const {
        return m_fallbackImage3D;
    }
    const Graphics::Image& get_fallback_cubemap() const {
        return m_fallbackCubemap;
    }

    // ----------------------------
    // Uniform Buffer Management
    // ----------------------------

    template <typename... UBOs>
    void register_UBO( const std::string& name ) {
        size_t           totalSize = ( pad_size<UBOs>() + ... );
        Graphics::Buffer buffer    = m_device->create_buffer_VMA(
            totalSize,
            BUFFER_USAGE_UNIFORM_BUFFER,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            static_cast<uint32_t>( totalSize ) );
        m_ubos[name] = buffer;
    }

    template <typename... UBOs>
    void register_UBO_array( const std::string& name, uint32_t count ) {
        std::vector<size_t> batchSizes = { pad_size<UBOs>()... };
        size_t              strideSize = 0;
        for ( size_t s : batchSizes )
            strideSize += s;

        Graphics::Buffer buffer = m_device->create_buffer_VMA(
            strideSize * count,
            BUFFER_USAGE_UNIFORM_BUFFER,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            static_cast<uint32_t>( strideSize ) );
        buffer.batchSizes = std::move( batchSizes );
        m_ubos[name]      = buffer;
    }

    const Graphics::Buffer& get_ubo( const std::string& name ) const {
        return m_ubos.at( name );
    }

    // -----------------------------------------------------
    // Image Registration — Raw Data
    // -----------------------------------------------------

    template <typename T>
    void register_image( const std::string&      name,
                         T*                      data,
                         Extent3D                extent,
                         int                     channels,
                         Graphics::ImageConfig   config        = {},
                         Graphics::SamplerConfig samplerConfig = {} ) {
        static_assert( std::is_arithmetic_v<T>, "register_image only supports arithmetic data types" );

        size_t bytes_per_pixel = sizeof( T ) * static_cast<size_t>( channels );

        Graphics::Image image;
        m_device->upload_texture_image( image, config, samplerConfig, static_cast<void*>( data ), bytes_per_pixel );
        m_images[name] = image;
    }

    // -----------------------------------------------------
    // Image Registration — From Texture
    // -----------------------------------------------------

    void register_image( const std::string&    name,
                         Core::ITexture* const t );

    const Graphics::Image& get_image_resource( const std::string& name ) const {
        return m_images.at( name );
    }

    // -----------------------------------------------------
    // Utility
    // -----------------------------------------------------
    static void upload_texture_data( const ptr<Graphics::Device>& device, Core::ITexture* const t );
    static void upload_geometry_data( const ptr<Graphics::Device>& device, Core::Geometry* const g, bool createAccelStructure = true );
    static void destroy_texture_data( Core::ITexture* const t );
    static void destroy_geometry_data( Core::Geometry* const g );
};

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END