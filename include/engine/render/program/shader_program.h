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
public:
    struct UniformBinding {
        uint32_t         set;
        uint32_t         binding;
        UniformType      type;
        ShaderStageFlags stages        = SHADER_STAGE_VERTEX | SHADER_STAGE_FRAGMENT;
        uint32_t         dynamicOffset = 0;
        bool             bindless      = false;
        BindingSource    source        = BindingSource::Attachment;
        std::string      resourceName  = "";
    };

    ShaderProgram( std::string name, std::string glslPath, const std::unordered_map<std::string, UniformBinding>& uniformBindings )
        : m_name( name )
        , m_shaderPath( glslPath )
        , m_uniformBindings( uniformBindings ) {
    }
    virtual ~ShaderProgram() = default;

    void attach( const std::string& uniformName, UniformResource& reosurce, Frame& frame );
    void attach( const std::string& uniformName, UniformResource& reosurce, uint32_t arraySlot, Frame& frame ); // For bindless

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

protected:
    std::vector<Graphics::DescriptorLayout> m_descriptorLayouts; // Uniform layouts

    std::unordered_map<std::string, UniformBinding> m_uniformBindings;
    std::string                                     m_name;
    std::string                                     m_shaderPath;

    bool m_compiled = false;

    friend class RenderGraph;

    void create_descriptor_layouts( const std::shared_ptr<Graphics::Device>& device );
};

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif