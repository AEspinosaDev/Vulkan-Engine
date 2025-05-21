/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDER_VIEW
#define RENDER_VIEW

#include <engine/common.h>
#include <engine/graphics/descriptors.h>
#include <engine/graphics/shaderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

class RenderGraph;

enum class UniformType
{
    ImageSampler,
    Buffer,
    DynamicBuffer,
    StorageBuffer,
    StorageImage,
    Sampler,
    // Add more as needed
};

class ShaderProgram
{
    Graphics::ShaderPass                    m_shaderpass;
    std::vector<Graphics::DescriptorLayout> m_descriptorLayouts; // Uniform layouts

    struct UniformBinding {
        uint32_t         set;
        uint32_t         binding;
        UniformType      type;
        ShaderStageFlags stages;
        std::string      name; // Optional, for debugging
    };

    std::vector<UniformBinding> m_uniformBindings;
    std::string                 m_name;
    // bool m_isCompute

    bool m_compiled = false;

    friend class RenderGraph;

public:
    ShaderProgram( std::string name, std::string glslPath, const UniformBinding& uniformBindings, const Graphics::GraphicPipelineSettings& settings = {} )
        : m_uniformBindings( uniformBindings ) {
    }

    const std::vector<UniformBinding>&       get_uniform_bindings() const { return m_uniformBindings };
    const std::string&                       get_glsl_path() const { return m_uniformBindings };
    const Graphics::GraphicPipelineSettings& get_settings() const { return m_uniformBindings };

    attach( const std::string& uName,
            const std::string& resName,
            Graphics::Frame&   frame ); // update descriptors
                                      // attach(const std::string& uniformName,variantResource)
                                      // update("uniform",std::variant<resource>, frame) updatedescriptors
    void bind_uniforms( uint32_t                     set,
                        Graphics::Frame&             frame,
                        const std::vector<uint32_t>& offsets = {} ); // if needed
                                                                     //  void bind_uniform(std::name, uintOffser if needede, Frame)
    void bind();
    bool compiled();

    bool cleanup();
    // Friend class Graph creates all shaderpasse (descritpor layouts builds modules etc)
};

// ShaderProgram lightingShader(
//     "lighting",
//     R"(
//         #version 450
//         layout(set = 0, binding = 0) uniform sampler2D albedo;
//         layout(set = 1, binding = 0) uniform CameraUBO { mat4 viewProj; };
//     )",
//     {
//         { 0, 0, DescriptorType::ImageSampler, STAGE_VERTEX | STAGE_FRAGMENT ,"albedo" },
//         { 1, 0, DescriptorType::UniformBuffer, "camera" }
//     },
//     GraphicsSettings{/* blend, cull, depth, etc. */}
// );

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif