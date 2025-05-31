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

    static ShaderSource read_file( const std::string& filePath );

    static std::vector<uint32_t> compile_shader( const std::string          src,
                                                 const std::string          shaderName,
                                                 shaderc_shader_kind        kind,
                                                 shaderc_optimization_level optimization );

    static ShaderStage
    create_shader_stage( VkDevice device, VkShaderStageFlagBits stageType, const std::vector<uint32_t> code );
};

/*
Base shader pass data structure
*/
struct BaseShaderPass {

    VkDevice         device         = VK_NULL_HANDLE;
    VkPipeline       pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    std::vector<PushConstant> pushConstants;

    BaseShaderPass() {}
    virtual void
                 compile_shader_stages( const std::string& src, shaderc_optimization_level optimization = shaderc_optimization_level_performance ) = 0;
    virtual void cleanup();
};
/*
Base shader pass data structure
*/
typedef BaseShaderPass ShaderPass;
/*
Standard ShaderPass for drawing purposes
*/
struct GraphicShaderPass final : public ShaderPass {

    std::vector<ShaderStage> shaderStages;
    GraphicPipelineConfig    config     = {};
    RenderPass*              renderpass = nullptr;
    Extent2D                 extent     = { 0, 0 };

    GraphicShaderPass()
        : ShaderPass() {}
    void compile_shader_stages( const std::string& src, shaderc_optimization_level optimization = shaderc_optimization_level_performance );
    void cleanup();
};

/*
ShaderPass for GPGPU
*/
struct ComputeShaderPass final : public ShaderPass {

    ShaderStage computeStage = {};

    ComputeShaderPass()
        : ShaderPass() {}
    void compile_shader_stages( const std::string& src, shaderc_optimization_level optimization = shaderc_optimization_level_performance );
    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif