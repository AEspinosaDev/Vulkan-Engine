/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAME_H
#define FRAME_H

#include <engine/graphics/command_buffer.h>
#include <engine/graphics/descriptors.h>
#include <engine/graphics/semaphore.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct Frame {
    // Control
    Semaphore presentSemaphore = {};
    Semaphore renderSemaphore  = {};
    Fence     renderFence      = {};
    // Command
    CommandPool   commandPool   = {};
    CommandBuffer commandBuffer = {};

    // GPU Global Buffers
    Buffer globalBuffer;
    Buffer objectBuffer;

    // Descriptors
    DescriptorPool descriptorPool;
    // Per-frame cache of descriptor sets: [ShaderProgram*, setIndex] -> VkDescriptorSet
    struct PairHash {
        template <typename T1, typename T2>
        std::size_t operator()( const std::pair<T1, T2>& p ) const {
            return std::hash<T1>()( p.first ) ^ ( std::hash<T2>()( p.second ) << 1 );
        }
    };
    std::unordered_map<std::pair<std::string, uint32_t>, DescriptorSet, PairHash> descriptorSets; // string shaderprogram name + set
    std::unordered_map<std::string, Buffer>                                       ubos;           // string resoruce name + actual buffer

    uint32_t index = 0;

    void cleanup();

    static bool guiEnabled;
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif