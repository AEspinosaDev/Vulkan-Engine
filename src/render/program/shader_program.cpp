#include <engine/render/program/shader_program.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

// void ShaderProgram::attach( const std::string& uniformName, UniformResource& resource, Frame& frame ) {
//     auto& descriptorSet = frame.m_descriptorSets[{ m_name, 0 }];
//     auto& uniformData   = m_uniformBindings[{ uniformName }];

//     if ( std::holds_alternative<Graphics::Buffer>( resource ) )
//     {
//         auto& buffer = std::get<Graphics::Buffer>( resource );
//         descriptorSet.update( buffer, 0, 0, uniformData.type, uniformData.binding );
//     } else if ( std::holds_alternative<Graphics::Texture>( resource ) )
//     {
//         auto& texture = std::get<Graphics::Texture>( resource );
//         descriptorSet.update( texture, IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uniformData.binding );
//     } else if ( std::holds_alternative<Graphics::TLAS>( resource ) )
//     {
//         auto& tlas = std::get<Graphics::TLAS>( resource );
//         descriptorSet.update( tlas, uniformData.binding );
//     }
// }
// void ShaderProgram::attach( const std::string& uniformName, UniformResource& reosurce, uint32_t arraySlot, Frame& frame ) {
// }

void ShaderProgram::create_descriptor_layouts( const std::shared_ptr<Graphics::Device>& device ) {

    // Setup descriptor layouts

    std::unordered_map<uint32_t, std::vector<Graphics::LayoutBinding>> sets;
    for ( auto& [name, uniform] : m_uniformBindings )
    {
        sets[uniform.set].push_back( { uniform.type, uniform.stages, uniform.binding, 1, uniform.bindless } );
    }
    m_descriptorLayouts.resize( sets.size() );

    uint16_t i = 0;
    for ( auto& [set, bindings] : sets )
    {
        m_descriptorLayouts[i] =
            device->create_descriptor_layout( bindings,
                                              0,
                                              bindings.size() == 1 && bindings.back().bindless ? VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT : 0 );
        i++;
    }
}

void ShaderProgram::cleanup() {
    for ( auto& layout : m_descriptorLayouts )
    {
        layout.cleanup();
    }
}

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END