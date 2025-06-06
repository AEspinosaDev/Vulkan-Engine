#include <engine/render/render_graph.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

std::string RenderGraphBuilder::create_target( const std::string& name, const TargetInfo& info ) {
    auto& r       = m_graph.get_or_create_resource( name );
    r.info        = info;
    r.written     = true;
    r.firstWriter = m_passIndex;

    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( name );

    return name;
}

std::string RenderGraphBuilder::read( const std::string& name, const ReadInfo& info ) {
    auto& res = m_graph.get_or_create_resource( name );

    res.readInfos[m_graph.m_passes[m_passIndex].name] = {};
    m_graph.m_passes[m_passIndex].readAttachmentKeys.push_back( name );

    return name;
}

void RenderGraphBuilder::write( const std::string& name ) {
    auto& r   = m_graph.get_or_create_resource( name );
    r.written = true;

    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( name );
}
} // namespace Render
VULKAN_ENGINE_NAMESPACE_END