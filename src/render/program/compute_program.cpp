#include <engine/render/program/compute_program.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

void ComputeShaderProgram::bind( Frame& frame ) {
    frame.m_commandBuffer.bind_shaderpass( m_shaderpass );
}

void ComputeShaderProgram::bind_uniform_set( uint32_t set, Frame& frame ) {
    auto& descriptorSet = frame.m_descriptorSets[{ m_name, set }];
    frame.m_commandBuffer.bind_descriptor_set( descriptorSet, set, m_shaderpass );
}

void ComputeShaderProgram::bind_uniform_set( uint32_t set, Frame& frame, const std::vector<uint32_t>& offsets ) {
    auto& descriptorSet = frame.m_descriptorSets[{ m_name, set }];
    frame.m_commandBuffer.bind_descriptor_set( descriptorSet, set, m_shaderpass, offsets );
}

void ComputeShaderProgram::cleanup() {
    ShaderProgram::cleanup();
    m_shaderpass.cleanup();
}

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END