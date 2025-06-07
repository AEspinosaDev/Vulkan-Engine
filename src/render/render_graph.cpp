#include <engine/render/render_graph.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

#pragma region Builder
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
#pragma region Reconciler

void RenderGraphReconciler::reconcile_attachments() {
}

void RenderGraphReconciler::reconcile_renderpasses() {
}

void RenderGraphReconciler::reconcile_shaders() {
}

#pragma region Graph

bool RenderGraph::reconcile( const RenderGraphResource& res, const Graphics::Image& image ) {
    auto& config = image.config;
    auto& info   = res.info;

    bool dirtyFBO = image.extent.width != info.extent.width &&
                    image.extent.height != info.extent.height &&
                    config.usageFlags != info.usage &&
                    config.format != info.format &&
                    config.layers != info.layers &&
                    config.mipmaps != info.mipmaps &&
                    config.samples != info.samples;

    //     bool dirtyRenderPass =

    m_renderCache[m_passes[res.firstWriter].name].validFBO = !dirtyFBO;
    return dirtyFBO;
}
bool RenderGraph::reconcileRead( const ReadInfo& info, const Graphics::Texture& texture ) {
    auto& config = texture.config;

    bool dirtyTex = config != info;

    return dirtyTex;
}
void RenderGraph::bake() {

    // Create of update images based on the current volatile
    // GraphResource Info
    for ( auto& [name, res] : m_transientResources )
    {
        auto& attachment = m_attachmentCache[name];
        bool  dirty      = reconcile( res, attachment.image );
        if ( !attachment.image.handle || dirty )
        {
            // Clean
            attachment.image.cleanup();
            // Create
            const auto& info = res.info;
            attachment.image = m_device->create_image( { info.extent.width, info.extent.height, 1 },
                                                       { .format     = info.format,
                                                         .usageFlags = info.usage,
                                                         .samples    = info.samples,
                                                         .mipmaps    = info.mipmaps,
                                                         .layers     = info.layers,
                                                         .clearValue = info.clearValue } );
        }
        // Check texture reads
        for ( auto& [name, read] : res.readInfos )
        {
            if ( !attachment.textures[name].image || dirty || reconcileRead( read, attachment.textures[name] ) )
            {
                attachment.textures[name].cleanup();
                attachment.textures[name] = m_device->create_texture( &attachment.image, read );
            }
        }
    }

    // For every pass info, check in renderpass is valid
    //  and compile shader programs and update uniforms
    for ( auto& pass : m_passes )
    {
        auto&      rt          = m_renderCache[pass.name];
        const auto NUM_TARGETS = pass.writeAttachmentsKeys.size();

        // IF NOT VALID RENDERPASS
        // -------------------------------
        if ( !rt.validRenderPass )
        {
            rt.renderPass.cleanup();

            std::vector<Graphics::RenderTargetInfo> targets;
            targets.resize( NUM_TARGETS );
            for ( size_t i = 0; i < NUM_TARGETS; i++ )
            {
                targets[i] = m_transientResources[pass.writeAttachmentsKeys[i]].info;
            }

            rt.renderPass = m_device->create_render_pass( targets, {} );
        }
        // IF NOT VALID FRAMEBUFFER
        // -------------------------
        if ( !rt.validFBO )
        {
            // Clean
            for ( auto& fbo : rt.fbos )
                fbo.cleanup();
            rt.fbos.clear();

            // Create
            std::vector<Graphics::Image*> fboAttachments;
            fboAttachments.resize( NUM_TARGETS );
            for ( size_t i = 0; i < NUM_TARGETS; i++ )
            {
                fboAttachments[i] = &m_attachmentCache[pass.writeAttachmentsKeys[i]].image;
            }
            // With pass and attachment info
            rt.fbos.push_back( m_device->create_framebuffer( rt.renderPass, fboAttachments ) );

            rt.validFBO = true;
        }

        // Compile and prepare shaders programs
        for ( const auto& shaderKey : pass.shaderProgramKeys )
        {
            auto& shader = m_shaderCache[shaderKey];
            if ( !shader->compiled() || !rt.validRenderPass )
            {
                shader->cleanup();
                shader->create_descriptor_layouts( m_device );
                shader->is_graphics() ? std::static_pointer_cast<GraphicShaderProgram>( shader )->m_shaderpass =
                                            m_device->create_graphic_shader_pass( shader->m_shaderPath,
                                                                                  shader->m_descriptorLayouts,
                                                                                  std::static_pointer_cast<GraphicShaderProgram>( shader )->m_shaderpass.config,
                                                                                  rt.renderPass )
                                      : std::static_pointer_cast<ComputeShaderProgram>( shader )->m_shaderpass =
                                            m_device->create_compute_shader_pass( shader->m_shaderPath,
                                                                                  shader->m_descriptorLayouts );
                shader->m_compiled = true;
            }

            // Update uniforms and allocate ALWAYS !!
            // Allocate
            for ( const auto& [name, binding] : shader->m_uniformBindings )
            {
                switch ( binding.source )
                {
                    case BindingSource::Attachment:
                        auto& tex  = m_attachmentCache[name].textures[pass.name];
                        auto& desc = m_frame->m_descriptorSets[{ shaderKey, binding.set }];
                        // IF there is no texture you use a dummy texture from shared resources
                        desc.update( tex, tex.config.expectedLayout, binding.binding, binding.type );
                        break;
                        // case BindingSource::Frame:
                        //     // auto& desc = m_frame->m_descriptorSets[{ shaderKey, binding.set }];
                        //     break;
                        // case BindingSource::Shared:
                        //     // auto& desc = m_frame->m_descriptorSets[{ shaderKey, binding.set }];
                        //     break;
                        // case BindingSource::Manual:
                        //     break;
                }
            }
        }

        rt.validRenderPass = true;
    }
}

void RenderGraph::transition_attachments( const RenderPassInfo& pass ) {
    // Perform layout transitions for images
    for ( auto& attachmentName : pass.readAttachmentKeys )
    {
        auto& tex = m_attachmentCache[attachmentName].textures[pass.name];
        m_frame->m_commandBuffer.pipeline_barrier( *tex.image, tex.image->currentLayout, tex.config.expectedLayout );
    };

    // for ( auto& resPair : m_transientResources )
    // {
    //     auto& res        = resPair.second;
    //     auto& layoutUses = res.layoutHistory;

    //     for ( size_t i = 1; i < layoutUses.size(); ++i )
    //     {
    //         if ( layoutUses[i].passName == pass.name )
    //         {
    //             auto& prev = layoutUses[i - 1];
    //             auto& next = layoutUses[i];

    //             if ( prev.layout != next.layout )
    //             {
    //                 auto& attachment = m_attachmentCache[res.name].image;
    //                 m_frame->m_commandBuffer.pipeline_barrier(
    //                     *attachment,
    //                     prev.layout,
    //                     next.layout,
    //                     prev.access,
    //                     next.access,
    //                     prev.stage,
    //                     next.stage );
    //             }

    //             break;
    //         }
    //     }
    // }
}

void RenderGraph::begin_frame( Frame& f ) {
    m_frame = &f;
    m_passes.clear();
    m_transientResources.clear();
    m_frame->wait();
}

int RenderGraph::add_pass( const std::string&                                                                   name,
                           const std::vector<std::string>&                                                      shaderPrograms,
                           std::function<void( RenderGraphBuilder& )>                                           onSetup,
                           std::function<void( const RenderView&, const Resources&, const RenderPassOutputs& )> onExecute ) {
    int id = static_cast<int>( m_passes.size() );
    m_passes.push_back( { name, onSetup, onExecute, shaderPrograms } );
    RenderGraphBuilder builder( *this, id );
    onSetup( builder );
    return id;
}

void RenderGraph::end_frame( const RenderView& view, const Resources& shared ) {
    // Baking
    bake();
    // Begin Command Buffer
    m_frame->start();
    // Execute Passes
    for ( auto& p : m_passes )
    {
        transition_attachments( p );

        p.executeCallback( view, shared, { m_renderCache[p.name].renderPass, m_renderCache[p.name].fbos } );
    }
    // End Command Buffer
    m_frame->end();
    // Submit workload
    m_frame->submit();
}

RenderGraphResource& RenderGraph::get_or_create_resource( const std::string& name ) {
    // Look if there is resource
    if ( !m_transientResources.count( name ) )
    {
        m_transientResources[name] = RenderGraphResource { .name = name };
        if ( !m_attachmentCache.count( name ) ) // If not, it creates the resource and a empty Image attachment bound to it
        {
            m_attachmentCache[name] = {};
        }
    }
    return m_transientResources[name];
}

} // namespace Render
// namespace Render
VULKAN_ENGINE_NAMESPACE_END