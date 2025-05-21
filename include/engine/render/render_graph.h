#ifndef RENDER_GRAPH
#define RENDER_GRAPH

#include <engine/common.h>
#include <engine/graphics/frame.h>
#include <engine/graphics/renderpass.h>
#include <engine/graphics/texture.h>
#include <engine/render/program/shader_program.h>
#include <engine/render/render_view.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

// Resource data
class RenderGraphResource
{
public:
    std::string                name;
    Graphics::RenderTargetInfo info;
    bool                       written     = false;
    int                        firstWriter = -1;
};

class RenderGraph;

class RenderGraphBuilder
{
public:
    RenderGraphBuilder( RenderGraph& g, int passID )
        : m_graph( g )
        , m_passIndex( passID ) {
    }

    std::string create_target( const std::string& name, const RenderTargetInfo& info );
    std::string read( const std::string& name );
    void        write( const std::string& name );

private:
    RenderGraph& m_graph;
    int          m_passIndex;
};

struct RenderPassInfo {
    std::string                                                                                name;
    std::function<void( RenderGraphBuilder& )>                                                 setupCallback;
    std::function<void( const RenderView&, const RenderResources&, const RenderPassOutputs& )> executeCallback;
    std::vector<std::string>                                                                   shaderProgramKeys;
    std::vector<std::string>                                                                   writeAttachmentsKeys;
    std::vector<std::string>                                                                   readAttachmentKeys;

    struct RuntimeData {
        Graphics::RenderPass               renderPass;
        std::vector<Graphics::Framebuffer> fbos;
        bool                               valid = false;
    } runtime;
};

class RenderGraph
{
private:
    ptr<Graphics::Device> m_device;
    Graphics::Frame*      m_frame = nullptr;

    std::vector<RenderPassInfo>                          m_passes;
    std::unordered_map<std::string, RenderGraphResource> m_transientResources; // Graph Resource (image ID)

    std::unordered_map<std::string, Graphics::Image>             m_attachmentCache; // Image attachments (image ID)
    std::unordered_map<std::string, Graphics::Texture>           m_textureCache;    //
    std::unordered_map<std::string, RenderPassInfo::RuntimeData> m_renderCache;     // Render runtime data (pass ID)
    std::unordered_map<std::string, ShaderProgram>               m_shaderCache;     // Pre-registered shaders (shader ID)

    void create_shader_program( const std::string& name );
    void create_render_pass( Graphics::RenderPass& renderpass );

    bool reconcile( const RenderTargetInfo& info, const Graphics::Image& imageAttachment ) {
        auto& config = imageAttachment.config;
        return imageAttachment.extent != info.extent &&
               config.usageFlags != info.usage &&
               config.format != info.format &&
               config.layers != info.layers &&
               config.mipmaps != info.mipmaps &&
               config.samples != info.samples;
    }
    void bake() {

        // Create of update images based on the current volatile
        // GraphResource Info
        for ( auto& [name, res] : m_transientResources )
        {
            auto&       attachment = m_attachmentCache[name];
            const auto& info       = res.info;
            if ( !attachment.handle || reconcile( info, attachment ) )
            {
                // Clean
                attachment.cleanup();
                // Create
                attachment = m_device->create_image( info.extent,
                                                     { .format     = info.format,
                                                       .usageFlags = info.usage,
                                                       .samples    = info.samples,
                                                       .mipmaps    = info.mipmaps,
                                                       .layers     = info.layers,
                                                       .clearValue = info.clearValue } );
                // Make Renderpass resource bound to image invalid
                m_renderCache[m_passes[res.firstWriter].name].valid = false;
            }
        }

        // For every pass info, check in renderpass is valid
        //  and compile shader programs and update uniforms
        for ( auto& pass : m_passes )
        {

            auto& rt = m_renderCache[pass.name];
            if ( !rt.valid )
            {
                // Clean
                rt.renderPass.cleanup();
                for ( auto& fbo : rt.fbos )
                {
                    fbo.cleanup();
                }
                rt.fbos.clear();

                // Create
                std::vector<Graphics::Image*>           fboAttachments;
                std::vector<Graphics::RenderTargetInfo> targets;
                const auto                              NUM_TARGETS = pass.writeAttachmentsKeys.size();
                fboAttachments.resize( NUM_TARGETS );
                targets.resize( NUM_TARGETS );
                for ( size_t i = 0; i < NUM_TARGETS; i++ )
                {
                    targets[i]        = m_transientResources[pass.writeAttachmentsKeys[i]].info;
                    fboAttachments[i] = m_attachmentCache[pass.writeAttachmentsKeys[i]];
                }
                // With pass and attachment info
                rt.renderPass = m_device->create_renderpass(targets,);

                rt.fbos.push_back( m_device->create_framebuffer( rt.renderPass, fboAttachments ) );

                rt.valid = true;
            }

            for ( const auto& shaderKey : pass.shaderProgramKeys )
            {
                auto& shader = m_shaderCache[shaderKey];
                if ( !shader.compiled() )
                {

                    // Setup shader program
                }

                // Update uniforms ALWAYS !!
            }
        }
    }

public:
    void begin_frame( const Graphics::Frame& f ) {
        m_frame = &f;
        m_passes.clear();
        m_transientResources.clear();
    }

    int add_pass( const std::string&                                                                         name,
                  const std::vector<std::string>&                                                            shaderPrograms,
                  std::function<void( RenderGraphBuilder& )>                                                 setupCallback,
                  std::function<void( const RenderView&, const RenderResources&, const RenderPassOutputs& )> execCallback ) {
        int id = static_cast<int>( passes.size() );
        passes.push_back( { name, setupCallback, execCallback, shaderPrograms } );
        RenderGraphBuilder builder( *this, id );
        setup( builder );
        return id;
    }

    void end_frame( const RenderView& view, const RenderResources& shared ) {
        // Baking
        bake();
        // Begin Command Buffer
        m_frame->commandBuffer.reset();
        m_frame->commandBuffer.begin();
        // Execute Passes
        for ( auto& p : m_passes )
        {
            RenderPassOutputs outputs = build_outputs( p.name );
            auto&             rt      = runtimeCache[p.name];
            p.execute( view, shared, outputs );
        }
        // End Command Buffer
        m_frame->commandBuffer.end();
    }

    RenderGraphResource& get_or_create_resource( const std::string& name ) {
        // Look if there is resource
        if ( !m_transientResources.count( name ) )
        {
            m_transientResources[name] = RenderGraphResource { .name = name };
            if ( !m_attachmentCache.count( name ) ) // If not, it creates the resource and a empty Image attachment bound to it
            {
                m_attachmentCache[name] = {};
            }
        }
        return transientResources[name];
    }

    // RuntimeResource& get_runtime_resource(const std::string& name) {
    //     return persistentResources.at(name);
    // }

    // ShaderProgram* get_shader_program(const std::string& name) {
    //     return shaderCache.count(name) ? &shaderCache[name] : nullptr;
    // }

    void register_shader( ShaderProgram&& shader ) {
        shaderCache[shader.name] = std::move( shader );
    }
};

std::string RenderGraphBuilder::create_target( const std::string& name, const RenderTargetInfo& info ) {
    auto& r       = m_graph.get_or_create_resource( name );
    r.info        = info;
    r.written     = true;
    r.firstWriter = m_passIndex;
    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( name );
    return name;
}

std::string RenderGraphBuilder::read( const std::string& name ) {
    m_graph.get_or_create_resource( name );
    m_graph.m_passes[m_passIndex].readAttachmentKeys.push_back( name );
    return name;
}

void RenderGraphBuilder::write( const std::string& name ) {
    auto& r       = m_graph.get_or_create_resource( name );
    r.written     = true;
    r.firstWriter = m_passIndex;
    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( name );
}

// class GBufferPass : public IRenderPass {
// public:
//     ShaderProgram* shader = nullptr;

//     GBufferPass(ShaderProgram* program) : shader(program) {}

//     void build(RenderGraphBuilder& builder) override {
//         builder.create_target("depth", RenderTargetInfo{
//             .format = VK_FORMAT_D32_SFLOAT,
//             .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//             .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
//             .clearValue = {.depthStencil = {1.0f, 0}},
//             .isDepth = true
//         });

//         builder.create_target("albedo", RenderTargetInfo{
//             .format = VK_FORMAT_R8G8B8A8_UNORM,
//             .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//             .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//             .clearValue = {.color = {{0, 0, 0, 1}}}
//         });
//     }

//     void execute(const RenderView& view, const FrameContext& frame, const RenderResources& shared, const RenderPassOutputs& outputs, ShaderProgram* program)
//     override {
//         // Bind pipeline, descriptor set, draw meshes
//     }
// };

// void exampleUsage(RenderGraph& graph, const RenderView& view, const FrameContext& frame, const RenderResources& shared) {
//     ShaderProgram gbufferShader("GBuffer");
//     ShaderProgram lightingShader("Lighting");

//     graph.register_shader(std::move(gbufferShader));
//     graph.register_shader(std::move(lightingShader));

//     graph.begin_frame(frame);

//     GBufferPass gpass(graph.get_shader_program("GBuffer"));
//     graph.add_pass("GBuffer", gpass.shader, [&](RenderGraphBuilder& b) { gpass.build(b); },
//                    [&](const RenderView& view, const RenderResources& shared, const RenderPassOutputs& outputs) {
//                        gpass.execute(view, frame, shared, outputs, gpass.shader);
//                    });

//     graph.add_pass("Lighting", graph.get_shader_program("Lighting"),
//         [&](RenderGraphBuilder& builder) {
//             builder.read("depth");
//             builder.read("albedo");
//             builder.create_target("lighting", RenderTargetInfo{
//                 .format = VK_FORMAT_R16G16B16A16_SFLOAT,
//                 .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//                 .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//                 .clearValue = {.color = {{0, 0, 0, 1}}},
//                 .load = false,
//                 .store = true
//             });
//         },
//         [&](const RenderView& view, const RenderResources& shared, const RenderPassOutputs& outputs) {
//             auto* program = graph.get_shader_program("Lighting");
//             // Bind pipeline and descriptor set, draw fullscreen quad
//         });

//     graph.end_frame(view, shared);
// }

// struct RenderPassOutputs {
//     std::unordered_map<std::string, VkImageView> views;
//     VkFramebuffer                                framebuffer = VK_NULL_HANDLE;

//     VkImageView get(const std::string& name) const {
//         auto it = views.find(name);
//         return it != views.end() ? it->second : VK_NULL_HANDLE;
//     }
// };

// class IRenderPass
// {
//   public:
//     virtual ~IRenderPass()                          = default;
//     virtual void build(RenderGraphBuilder& builder) = 0;
//     virtual void
//     execute(const RenderView& view, const FrameContext& frame, const RenderResources& shared, const RenderPassOutputs& outputs, ShaderProgram* program) = 0;
// };

//     RenderPassOutputs build_outputs(const std::string& passName) {
//         RenderPassOutputs outputs;
//         for (auto& [name, res] : transientResources)
//         {
//             if (res.firstWriter == -1 || passes[res.firstWriter].name != passName)
//                 continue;
//             outputs.views[name] = persistentResources[name].view;
//         }
//         return outputs;
//     }

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END

#endif
