#include <engine/render/program/shader_program.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

void ShaderProgram::attach( const std::string& uniformName, Frame& frame ) {
    auto& descriptorSet = frame.m_descriptorSets[{ m_name, 0 }];
    descriptorSet.update(frame.m_ubos[uniformName],)
}
void ShaderProgram::attach( const std::string& uniformName, UniformResource& resource, Frame& frame ) {
}
void ShaderProgram::attach( const std::string& uniformName, UniformResource& reosurce, uint32_t arraySlot, Frame& frame ) {
}

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END