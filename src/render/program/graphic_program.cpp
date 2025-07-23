#include <engine/render/program/graphic_program.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

void GraphicShaderProgram::compile( const std::shared_ptr<Graphics::Device>& device ) {

    ShaderProgram::create_descriptor_layouts( device );
    m_shaderpass = device->create_graphic_shader_pass( m_shaderPath, m_descriptorLayouts, m_shaderpass.config, m_renderpass );
    // m_shaderpass = std::move( device->create_graphic_shader_pass( m_shaderPath, m_descriptorLayouts, m_shaderpass.config, m_renderpass ) );
    m_compiled   = true;
}
void GraphicShaderProgram::bind( Frame& frame ) {

    frame.m_commandBuffer.bind_shaderpass( m_shaderpass );
}

void GraphicShaderProgram::bind_uniform_set( uint32_t set, Frame& frame ) {
    auto& descriptorSet = frame.m_descriptorSets[{ m_name, set }];
    frame.m_commandBuffer.bind_descriptor_set( descriptorSet, set, m_shaderpass );
}

void GraphicShaderProgram::bind_uniform_set( uint32_t set, Frame& frame, const std::vector<uint32_t>& offsets ) {
    auto& descriptorSet = frame.m_descriptorSets[{ m_name, set }];
    frame.m_commandBuffer.bind_descriptor_set( descriptorSet, set, m_shaderpass, offsets );
}

void GraphicShaderProgram::cleanup() {
    ShaderProgram::cleanup();
    m_shaderpass.cleanup();
}

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END