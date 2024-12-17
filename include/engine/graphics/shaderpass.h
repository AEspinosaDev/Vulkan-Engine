/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SHADER_H
#define SHADER_H

#include <engine/graphics/pipeline.h>
#include <engine/graphics/renderpass.h>
#include <engine/graphics/utilities/translator.h>

#include <unordered_map>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct ShaderStage {
    VkShaderModule        shaderModule;
    VkShaderStageFlagBits stage;
};
/*
/Shader useful info
*/
struct ShaderSource {
    std::string name;

    std::string vertSource;
    std::string fragSource;
    std::string geomSource;
    std::string tessControlSource;
    std::string tessEvalSource;
    std::string computeSource;

    static ShaderSource read_file(const std::string& filePath);

    static std::vector<uint32_t> compile_shader(const std::string          src,
                                                const std::string          shaderName,
                                                shaderc_shader_kind        kind,
                                                shaderc_optimization_level optimization);

    static ShaderStage
    create_shader_stage(VkDevice device, VkShaderStageFlagBits stageType, const std::vector<uint32_t> code);
};
/*
Base shader pass data structure
*/
struct BaseShaderPass {

    const QueueType QUEUE_TYPE;
    std::string     filePath;

    VkDevice         device         = VK_NULL_HANDLE;
    VkPipeline       pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    PipelineSettings settings = {};

    BaseShaderPass(VkDevice _device, const std::string shaderFile, QueueType type)
        : filePath(shaderFile)
        , device(_device)
        , QUEUE_TYPE(type) {
    }

    virtual void
    build_shader_stages(shaderc_optimization_level optimization = shaderc_optimization_level_performance) = 0;
    virtual void build(DescriptorPool& descriptorManager)                                                 = 0;
    virtual void cleanup();
};
/*
Base shader pass data structure
*/
typedef BaseShaderPass ShaderPass;
/*
Standard ShaderPass for drawing purposes
*/
struct GraphicShaderPass : public ShaderPass {

    std::vector<ShaderStage> shaderStages;
    GraphicPipelineSettings  graphicSettings = {};
    RenderPass*              renderpass      = nullptr;
    Extent2D                 extent          = {0, 0};

    GraphicShaderPass(VkDevice _device, RenderPass& renderPass, Extent2D _extent, const std::string shaderFile)
        : ShaderPass(_device, shaderFile, GRAPHIC_QUEUE)
        , renderpass(&renderPass)
        , extent(_extent) {
    }

    void build_shader_stages(shaderc_optimization_level optimization = shaderc_optimization_level_performance);

    void build(DescriptorPool& descriptorManager);

    void cleanup();
};

/*
ShaderPass for GPGPU
*/
struct ComputeShaderPass : public ShaderPass {
    ShaderStage computeStage = {};

    ComputeShaderPass(VkDevice _device, const std::string shaderFile)
        : ShaderPass(_device, shaderFile, COMPUTE_QUEUE) {
    }

    void build_shader_stages(shaderc_optimization_level optimization = shaderc_optimization_level_performance);

    void build(DescriptorPool& descriptorManager);

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif