/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SHADER_PROGRAM
#define SHADER_PROGRAM

#include <engine/common.h>
#include <engine/graphics/device.h>
#include <engine/graphics/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

class RenderGraph;

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

        UniformBinding( uint32_t set, uint32_t binding, UniformType type, ShaderStageFlags stages, std::string name ) {}
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

    void attach( const std::string& uName,
                 Graphics::Frame&   frame );
    // void attach( const std::string& uName, std::variant<Buffer, > reosurce     );
    // void attach( const std::string& uName,
    //             std::variant<Buffer, > reosurce
    //              uint32_t arraySlot);

    // update descriptors
    // attach(const std::string& uniformName,variantResource)
    // update("uniform",std::variant<resource>, frame) updatedescriptors
    void bind_uniforms( uint32_t                     set,
                        Graphics::Frame&             frame,
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

class GraphicShaderProgram final : public ShaderProgram
{
    Graphics::GraphicShaderPass m_shaderpass = {};

public:
    GraphicShaderProgram( std::string name, std::string glslPath, const std::vector<UniformBinding>& uniformBindings, const Graphics::GraphicPipelineSettings& settings = {} )
        : ShaderProgram( name, glslPath, uniformBindings ) {
        m_shaderpass.settings = settings
    }

   void compile( const std::shared_ptr<Graphics::Device>& device ) override;
   inline bool is_graphics() const   override  { return true;}                                   
   bool is_compute() const    override { { return false;}   }                                     
   void cleanup()   override;                                               

    const Graphics::GraphicPipelineSettings& get_settings() const { return m_shaderpass.graphicSettings; }
};

class ComputeShaderProgram final : public ShaderProgram
{
    Graphics::ComputeShaderPass m_shaderpass = {};

public:
    ComputeShaderProgram( std::string name, std::string glslPath, const std::vector<UniformBinding>& uniformBindings )
        : ShaderProgram( name, glslPath, uniformBindings ) {
    }

   void compile( const std::shared_ptr<Graphics::Device>& device ) override;
   inline bool is_graphics() const   override  { return true;}                                   
   bool is_compute() const    override { { return false;}   }                                     
   void cleanup()   override;                                               

   
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