#include <engine/graphics/shaderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

ShaderStage
ShaderSource::create_shader_stage(VkDevice device, VkShaderStageFlagBits stageType, const std::vector<uint32_t> code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(unsigned int);
    createInfo.pCode    = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    ShaderStage stage;
    stage.shaderModule = shaderModule;
    stage.stage        = stageType;

    return stage;
}

ShaderSource ShaderSource::read_file(const std::string& filePath) {

    std::ifstream stream(filePath);
    std::string   scriptsPath(ENGINE_RESOURCES_PATH "shaders/include/");

    std::string       line;
    std::stringstream ss[6];
    enum class StageType
    {
        NONE            = -1,
        VERTEX          = 0,
        FRAGMENT        = 1,
        GEOMETRY        = 2,
        TESS_CONTROL    = 3,
        TESS_EVALUATION = 4,
        COMPUTE         = 5,
        ALL_STAGES      = 6
    };
    StageType type = StageType::NONE;

    while (getline(stream, line))
    {
        // SELECT THE SHADER STAGE
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = StageType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = StageType::FRAGMENT;
            else if (line.find("geometry") != std::string::npos)
                type = StageType::GEOMETRY;
            else if (line.find("tesscontrol") != std::string::npos)
                type = StageType::TESS_CONTROL;
            else if (line.find("tesseval") != std::string::npos)
                type = StageType::TESS_EVALUATION;
            else if (line.find("compute") != std::string::npos)
                type = StageType::COMPUTE;
        }
        // CHECK MODULES INCLUDED
        else if (line.find("#include") != std::string::npos)
        {
            size_t      start       = line.find(" ") + 1;
            std::string includeFile = Utils::trim(line.substr(start)); // Extract the file name and trim any spaces

            std::string fullIncludePath = scriptsPath + includeFile;

            std::string includeContent = Utils::read_file(fullIncludePath);
            ss[(int)type] << includeContent << '\n';
        }
        // FILL THE ACTUAL STAGE CODE
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return {filePath, ss[0].str(), ss[1].str(), ss[2].str(), ss[3].str(), ss[4].str(), ss[5].str()};
}

std::vector<uint32_t> ShaderSource::compile_shader(const std::string          src,
                                                   const std::string          shaderName,
                                                   shaderc_shader_kind        kind,
                                                   shaderc_optimization_level optimization) {
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetTargetSpirv(shaderc_spirv_version_1_6);

    options.SetOptimizationLevel(optimization);

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(src, kind, shaderName.c_str(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        DEBUG_LOG("Error compiling module - " << result.GetErrorMessage());
    }

    std::vector<uint32_t> spirv = {result.cbegin(), result.cend()};

    return spirv;
}

void GraphicShaderPass::build_shader_stages(shaderc_optimization_level optimization) {
    if (filePath == "")
        return;
    auto shader = ShaderSource::read_file(filePath);

    if (shader.vertSource != "")
    {
        ShaderStage vertShaderStage = ShaderSource::create_shader_stage(
            device,
            VK_SHADER_STAGE_VERTEX_BIT,
            ShaderSource::compile_shader(shader.vertSource, shader.name + "vert", shaderc_vertex_shader, optimization));
        shaderStages.push_back(vertShaderStage);
    }
    if (shader.fragSource != "")
    {
        ShaderStage fragShaderStage = ShaderSource::create_shader_stage(
            device,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            ShaderSource::compile_shader(
                shader.fragSource, shader.name + "frag", shaderc_fragment_shader, optimization));
        shaderStages.push_back(fragShaderStage);
    }
    if (shader.geomSource != "")
    {
        ShaderStage geomShaderStage = ShaderSource::create_shader_stage(
            device,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            ShaderSource::compile_shader(
                shader.geomSource, shader.name + "geom", shaderc_geometry_shader, optimization));
        shaderStages.push_back(geomShaderStage);
    }
    if (shader.tessControlSource != "")
    {
        ShaderStage tessControlShaderStage = ShaderSource::create_shader_stage(
            device,
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            ShaderSource::compile_shader(
                shader.tessControlSource, shader.name + "control", shaderc_tess_control_shader, optimization));
        shaderStages.push_back(tessControlShaderStage);
    }
    if (shader.tessEvalSource != "")
    {
        ShaderStage tessEvalShaderStage = ShaderSource::create_shader_stage(
            device,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            ShaderSource::compile_shader(
                shader.tessEvalSource, shader.name + "eval", shaderc_tess_evaluation_shader, optimization));
        shaderStages.push_back(tessEvalShaderStage);
    }
}
void ComputeShaderPass::build_shader_stages(shaderc_optimization_level optimization) {
    if (filePath == "")
        return;
    auto shader = ShaderSource::read_file(filePath);

    if (shader.computeSource != "")
    {
        computeStage = ShaderSource::create_shader_stage(
            device,
            VK_SHADER_STAGE_COMPUTE_BIT,
            ShaderSource::compile_shader(
                shader.computeSource, shader.name + "compute", shaderc_compute_shader, optimization));
    }
}

void GraphicShaderPass::build(DescriptorPool& descriptorManager) {
    PipelineBuilder::build_pipeline_layout(pipelineLayout, device, descriptorManager, settings);

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    for (auto& stage : shaderStages)
    {
        stages.push_back(Init::pipeline_shader_stage_create_info(stage.stage, stage.shaderModule));
    }
    PipelineBuilder::build_graphic_pipeline(
        pipeline, pipelineLayout, device, renderpass->handle, extent, graphicSettings, stages);
}
void ComputeShaderPass::build(DescriptorPool& descriptorManager) {

    PipelineBuilder::build_pipeline_layout(pipelineLayout, device, descriptorManager, settings);

    PipelineBuilder::build_compute_pipeline(
        pipeline,
        pipelineLayout,
        device,
        Init::pipeline_shader_stage_create_info(computeStage.stage, computeStage.shaderModule));
}
void GraphicShaderPass::cleanup() {

    for (auto& stage : shaderStages)
    {
        vkDestroyShaderModule(device, stage.shaderModule, nullptr);
    }
    ShaderPass::cleanup();
}
void ComputeShaderPass::cleanup() {

    vkDestroyShaderModule(device, computeStage.shaderModule, nullptr);
    ShaderPass::cleanup();
}

void BaseShaderPass::cleanup() {
    if (pipelineLayout)
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
    if (pipeline)
    {
        vkDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END