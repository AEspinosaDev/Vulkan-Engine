/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SHADER_PROGRAM
#define SHADER_PROGRAM

#include <engine/common.h>
#include <engine/render/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

class RenderGraph;

using UniformResource = std::variant<Graphics::Buffer, Graphics::Texture, Graphics::Accel>;

class ShaderProgram
{
protected:
    std::vector<Graphics::DescriptorLayout> m_descriptorLayouts; // Uniform layouts

    struct UniformBinding {
        uint32_t         set;
        uint32_t         binding;
        UniformType      type;
        ShaderStageFlags stages;
        std::string      name;
        bool             bindless = false;

        // UniformBinding( uint32_t set, uint32_t binding, UniformType type, ShaderStageFlags stages, std::string name ) {}
        // UniformBinding( UniformType type, ShaderStageFlags stages, std::string name ) {}
    };

    std::vector<UniformBinding> m_uniformBindings;
    std::string                 m_name;
    std::string                 m_shaderPath;

    bool m_compiled = false;

    friend class RenderGraph;

public:
    ShaderProgram( std::string name, std::string glslPath, const std::vector<UniformBinding>& uniformBindings )
        : m_name( name )
        , m_shaderPath( glslPath )
        , m_uniformBindings( uniformBindings ) {
    }
    virtual ~ShaderProgram() = default;

    void attach( const std::string& uniformName,
                 Frame&             frame );
    void attach( const std::string& uniformName, UniformResource& resource, Frame& frame );
    void attach( const std::string& uniformName, UniformResource& reosurce, uint32_t arraySlot, Frame& frame );

    void bind_uniforms( uint32_t                     set,
                        Frame&                       frame,
                        const std::vector<uint32_t>& offsets = {} ); // if needed
                                                                     //  void bind_uniform(std::name, uintOffser if needede, Frame)
    void bind();
    bool compiled();

    virtual void compile( const std::shared_ptr<Graphics::Device>& device ) = 0;
    virtual bool is_graphics() const                                        = 0;
    virtual bool is_compute() const                                         = 0;
    virtual void cleanup()                                                  = 0;

    const std::vector<UniformBinding>& get_uniform_bindings() const { return m_uniformBindings; }
    const std::string&                 get_shader_source_path() const { return m_shaderPath; }
    const std::string&                 get_name() const { return m_name; }
};

// ShaderProgram lightingShader(
//     "lighting",
//     R"(
//         #version 450
//         layout(set = 0, binding = 0) uniform sampler2D albedo;
//         layout(set = 1, binding = 0) uniform CameraUBO { mat4 viewProj; };
//     )",
//     {
//         { 0, 0, ImageSampler, STAGE_VERTEX | STAGE_FRAGMENT ,"albedo" },
//         { 1, 0, UniformBuffer, "camera" }
//     },
//     GraphicsSettings{/* blend, cull, depth, etc. */}
// );

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif