#include <engine/render/render_graph.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

#pragma region Builder
std::string RenderGraphBuilder::create_target( const std::string& name, const TargetInfo& info ) {
    auto& r       = m_graph.get_or_create_resource( name );
    r.info        = info;
    r.writeFBO    = 0;
    r.firstWriter = m_passIndex;

    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( name );

    return name;
}

std::string RenderGraphBuilder::create_depth_target( const std::string& name, Extent2D size, FloatPrecission precission, uint32_t layers ) {
    auto& r = m_graph.get_or_create_resource( name );
    r.info  = Render::TargetInfo {
         .extent      = size,
         .format      = precission == FloatPrecission::F16 ? DEPTH_16F : DEPTH_32F,
         .usage       = IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
         .finalLayout = LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         .clearValue  = { .depthStencil.depth = 1.0f },
         .load        = false,
         .store       = true,
         .layers      = layers };
    r.writeFBO    = 0;
    r.firstWriter = m_passIndex;

    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( name );

    return name;
}

std::string
RenderGraphBuilder::create_swapchain_target() {
    auto& r         = m_graph.get_or_create_resource( SWAPCHAIN_KEY );
    auto  swp       = m_graph.m_device->get_swapchain();
    auto  swpImage0 = swp.get_present_images()[0];
    r.info          = Render::TargetInfo {
                 .extent      = { swpImage0.extent.width, swpImage0.extent.height },
                 .format      = swpImage0.config.format,
                 .usage       = IMAGE_USAGE_COLOR_ATTACHMENT,
                 .finalLayout = LAYOUT_PRESENT,
                 .load        = false,
                 .store       = true,
                 .layers      = 1 };
    r.writeFBO    = 0;
    r.firstWriter = m_passIndex;

    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( SWAPCHAIN_KEY );
}
std::string RenderGraphBuilder::read( const std::string& name, const ReadInfo& info ) {
    auto& res = m_graph.get_or_create_resource( name );

    res.readInfos[m_graph.m_passes[m_passIndex].name] = {};
    m_graph.m_passes[m_passIndex].readAttachmentKeys.push_back( name );
    // res.layoutHistory.push_back({passName, info.expectedLayout, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, false});

    return name;
}

void RenderGraphBuilder::write( const std::string& name ) {
    auto& r = m_graph.get_or_create_resource( name );
    // r.written = true;

    // res.layoutHistory.push_back( { passName, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, true } );
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

        if ( name == SWAPCHAIN_KEY ) // If swapchain target that is all
            continue;

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
            // With pass and attachment info (cehck if its swapchain)
            const bool IS_SWAPCHAIN = pass.writeAttachmentsKeys[0] == SWAPCHAIN_KEY;
            if ( !IS_SWAPCHAIN )
                rt.fbos.push_back( m_device->create_framebuffer( rt.renderPass, fboAttachments ) );
            else
            {
                auto swp       = m_device->get_swapchain();
                auto swpImages = swp.get_present_images();
                for ( size_t i = 0; i < swpImages.size(); i++ )
                {
                    rt.fbos.push_back( m_device->create_framebuffer( rt.renderPass, { &swpImages[i] } ) );
                }
            }

            rt.validFBO = true;
        }

        // Compile and prepare shaders programs
        for ( const auto& shaderKey : pass.shaderProgramKeys )
        {
            auto& shader = m_shaderCache[shaderKey];
            if ( !shader->compiled() || !rt.validRenderPass )
            {
                shader->cleanup();
                if ( shader->is_graphics() )
                {
                    std::static_pointer_cast<GraphicShaderProgram>( shader )->set_renderpass(rt.renderPass);
                }
                shader->compile( m_device );
            }

            // Update uniforms and allocate ALWAYS !!
            // Allocate
            for ( const auto& [name, binding] : shader->m_uniformBindings )
            {
                switch ( binding.source )
                {
                    case BindingSource::Attachment:
                        auto& tex  = m_attachmentCache[binding.resourceName].textures[pass.name];
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

int RenderGraph::add_pass( const std::string&                                                                           name,
                           const std::vector<std::string>&                                                              shaderPrograms,
                           std::function<void( RenderGraphBuilder& )>                                                   onSetup,
                           std::function<void( const RenderView&, Frame&, const Resources&, const RenderPassOutputs& )> onExecute ) {
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
        p.executeCallback( view, *m_frame, shared, { m_renderCache[p.name].renderPass, m_renderCache[p.name].fbos } );
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