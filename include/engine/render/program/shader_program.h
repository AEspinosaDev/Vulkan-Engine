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
        ShaderStageFlags stages        = SHADER_STAGE_VERTEX | SHADER_STAGE_FRAGMENT;
        uint32_t         dynamicOffset = 0;
        bool             bindless      = false;
    };

    std::unordered_map<std::string, UniformBinding> m_uniformBindings;
    std::string                                     m_name;
    std::string                                     m_shaderPath;

    bool m_compiled = false;

    friend class RenderGraph;

    void create_descriptor_layouts( const std::shared_ptr<Graphics::Device>& device );

public:
    ShaderProgram( std::string name, std::string glslPath, const std::unordered_map<std::string, UniformBinding>& uniformBindings )
        : m_name( name )
        , m_shaderPath( glslPath )
        , m_uniformBindings( uniformBindings ) {
    }
    virtual ~ShaderProgram() = default;

    void attach( const std::string& uniformName, Graphics::Buffer& resource, Frame& frame );
    void attach( const std::string& uniformName, Graphics::Texture& resource, Frame& frame );
    void attach( const std::string& uniformName, Graphics::Accel& resource, Frame& frame );
    void attach( const std::string& uniformName, Graphics::Buffer& resource, uint32_t arraySlot, Frame& frame );
    void attach( const std::string& uniformName, Graphics::Texture& resource, uint32_t arraySlot, Frame& frame );
    void attach( const std::string& uniformName, Graphics::Accel& resource, uint32_t arraySlot, Frame& frame );
    // void attach( const std::string& uniformName, UniformResource& reosurce, Frame& frame );
    // void attach( const std::string& uniformName, UniformResource& reosurce, uint32_t arraySlot, Frame& frame );

    virtual void compile( const std::shared_ptr<Graphics::Device>& device )                           = 0;
    virtual void bind( Frame& frame )                                                                 = 0;
    virtual void bind_uniform_set( uint32_t set, Frame& frame )                                       = 0;
    virtual void bind_uniform_set( uint32_t set, Frame& frame, const std::vector<uint32_t>& offsets ) = 0; // if needed
    virtual bool is_graphics() const                                                                  = 0;
    virtual bool is_compute() const                                                                   = 0;

    virtual void cleanup();

    const std::unordered_map<std::string, UniformBinding>& get_uniform_bindings() const { return m_uniformBindings; }
    const std::string&                                     get_shader_source_path() const { return m_shaderPath; }
    const std::string&                                     get_name() const { return m_name; }
    inline bool                                            compiled() { return m_compiled; }
};

// ShaderProgram lightingShader(
//     "lighting",
//     R"(
//         #version 450
//         layout(set = 0, binding = 0) uniform sampler2D albedo;
//         layout(set = 1, binding = 0) uniform CameraUBO { mat4 viewProj; };
//     )",
//     {
//        {"iInoput", { 0, 0, ImageSampler, STAGE_VERTEX | STAGE_FRAGMENT }},
//        {"cameraUniforms", { 1, 0, UniformBuffer, "camera" }}
//     },
//     GraphicsSettings{/* blend, cull, depth, etc. */}
// );

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif