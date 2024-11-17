/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SHADER_H
#define SHADER_H

#include <engine/graphics/pipeline.h>
#include <engine/graphics/vk_renderpass.h>

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

    static ShaderSource read_file(const std::string& filePath);

    static std::vector<uint32_t> compile_shader(const std::string          src,
                                                const std::string          shaderName,
                                                shaderc_shader_kind        kind,
                                                shaderc_optimization_level optimization);

    static ShaderStage
    create_shader_stage(VkDevice device, VkShaderStageFlagBits stageType, const std::vector<uint32_t> code);
};

class ShaderPass
{
    const std::string        SHADER_FILE;
    std::vector<ShaderStage> m_shaderStages;

    VkDevice         m_device;
    VkPipeline       m_pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

  public:
    PipelineSettings settings{};

    ShaderPass(VkDevice device, const std::string shaderFile)
        : SHADER_FILE(shaderFile)
        , m_device(device) {
    }
    ShaderPass(VkDevice device, const std::string shaderFile, PipelineSettings sett)
        : SHADER_FILE(shaderFile)
        , settings(sett)
        , m_device(device) {
    }

    inline VkPipeline get_pipeline() const {
        return m_pipeline;
    }
    inline VkPipelineLayout get_layout() const {
        return m_pipelineLayout;
    }
    void build_shader_stages(shaderc_optimization_level optimization = shaderc_optimization_level_performance);
    void build(VulkanRenderPass renderPass, DescriptorPool& descriptorManager);
    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif