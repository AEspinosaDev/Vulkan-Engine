#include <engine/render/program/graphic_program.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

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

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END