/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDER_VIEW
#define RENDER_VIEW

#include <engine/common.h>
#include <engine/graphics/accel.h>
#include <engine/graphics/image.h>
#include <engine/graphics/vao.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

struct DrawParameters {
    Topology    topology    = Topology::TRIANGLES;
    bool        depthWrites = true;
    bool        depthTest   = true;
    CullingMode culling     = CullingMode::NO_CULLING;
};

struct DrawCall {
    Graphics::VAO  vertexArrays;
    uint32_t       indexOffset; // If multiple materials
    DrawParameters params;
    std::string    shaderID;
    uint32_t       bufferOffset; // Index in object uniform buffer
    bool           culled;

    struct BindedTexture {
        Graphics::Image* img     = nullptr;
        uint32_t         binding = 0;
    };
    
    std::vector<BindedTexture> textureBatch;
};

struct RenderView {
    /*Index*/
    uint32_t frameIndex        = 0;
    uint32_t presentImageIndex = 0;
    /*Draw Calls*/
    std::vector<DrawCall> drawCalls;
    std::vector<uint32_t> opaqueDrawCalls;
    std::vector<uint32_t> shadowDrawCalls;
    std::vector<uint32_t> transparentDrawCalls;

    uint32_t enviromentDrawCall = 0;
    bool     updateEnviroment   = false;
    /*Top Level AS*/
    Graphics::TLAS* TLAS        = nullptr;
    Graphics::TLAS* dynamicTLAS = nullptr;
    /*Number of shadow casting lights*/
    uint32_t numLights = 0;
    /*UBOs*/
    Graphics::Buffer globalBuffer;
    Graphics::Buffer objectBuffer;
    /*CMD*/
    Graphics::CommandBuffer commandBuffer;
};
} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif