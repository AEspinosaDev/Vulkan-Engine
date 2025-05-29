/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAME_H
#define FRAME_H

#include <engine/graphics/device.h>
#include <engine/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

class RenderGraph;
class ShaderProgram;
class RenderViewBuilder;

class Frame
{
    // Device PTR
    std::shared_ptr<Graphics::Device> m_device = nullptr;

    // Control
    Graphics::Semaphore m_presentSemaphore = {};
    Graphics::Semaphore m_renderSemaphore  = {};
    Graphics::Fence     m_renderFence      = {};
    // Command
    Graphics::CommandPool   m_commandPool   = {};
    Graphics::CommandBuffer m_commandBuffer = {};

    // Descriptors
    Graphics::DescriptorPool                                                                m_descriptorPool;
    std::unordered_map<std::pair<std::string, uint32_t>, Graphics::DescriptorSet, PairHash> m_descriptorSets; // string shaderprogram name + set
    // UBOS
    std::unordered_map<std::string, Graphics::Buffer> m_ubos; // string resoruce name + actual buffer

    uint32_t m_index = 0;

    friend class RenderGraph;       // For allocating descriptors
    friend class ShaderProgram;     // For updating descriptors
    friend class RenderViewBuilder; // For updating UBOs

    template <typename UBO>
    size_t pad_size() const {
        return m_device->pad_uniform_buffer_size( sizeof( UBO ) );
    }

public:
    uint32_t                       get_index() const { return m_index; }
    const Graphics::CommandBuffer& get_command_buffer() const { return m_commandBuffer; }
    const Graphics::Semaphore&     get_render_semaphore() const { return m_renderSemaphore; }
    const Graphics::Fence&         get_render_fence() const { return m_renderFence; }
    const Graphics::Semaphore&     get_present_semaphore() const { return m_presentSemaphore; }

    void init( const std::shared_ptr<Graphics::Device>& device, uint32_t index = 0 );
    // Waits for the render fence that was signaled
    void wait();
    // Resets control objects and begins the command buffer
    void start();
    // Ends the command buffer
    void end();
    // Submit command buffer workload
    void submit();

    void cleanup();

    // Uniform Buffer Managment
    // ---------------------------
    
    template <typename... UBOs>
    inline void register_UBO( const std::string& name ) {
        size_t           totalSize = ( pad_size<UBOs>() + ... );
        Graphics::Buffer buffer    = m_device->create_buffer_VMA(
            totalSize,
            BUFFER_USAGE_UNIFORM_BUFFER,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            (uint32_t)totalSize );
        m_ubos[name] = buffer;
    }
    template <typename... UBOs>
    inline void register_UBO_array( const std::string& name, uint32_t count ) {
        size_t           strideSize = ( pad_size<UBOs>() + ... );
        Graphics::Buffer buffer     = m_device->create_buffer_VMA(
            strideSize * count,
            BUFFER_USAGE_UNIFORM_BUFFER,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            (uint32_t)strideSize );
        m_ubos[name] = buffer;
    }

    const Graphics::Buffer& get_ubo( const std::string& name ) const {
        return m_ubos.at( name );
    }
};

struct PairHash {
    std::size_t operator()( const std::pair<std::string, uint32_t>& key ) const {
        std::size_t seed = 0;
        Utils::hash_combine( seed, std::hash<std::string> {}( key.first ) );
        Utils::hash_combine( seed, std::hash<uint32_t> {}( key.second ) );
        return seed;
    }
};

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END
#endif