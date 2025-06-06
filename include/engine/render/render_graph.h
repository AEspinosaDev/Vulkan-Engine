#ifndef RENDER_GRAPH
#define RENDER_GRAPH

#include <engine/common.h>
#include <engine/graphics/device.h>
#include <engine/graphics/renderpass.h>
#include <engine/graphics/texture.h>
#include <engine/render/frame.h>
#include <engine/render/program/shader_program.h>
#include <engine/render/render_view_builder.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

// Resource data
using TargetInfo = Graphics::RenderTargetInfo;
using ReadInfo   = Graphics::TextureConfig;

class RenderGraphResource
{
public:
    std::string name;
    TargetInfo  info;
    bool        written     = false;
    int         firstWriter = -1;
    // std::unordered_map<std::string, ReadInfo> readInfos; // pass name
    // std::vector<int> readers;
};

class RenderGraphAttachment
{
public:
    Graphics::Image                                    image = {};
    std::unordered_map<std::string, Graphics::Texture> textures; // pass name
    bool                                               used = false;
};

class RenderGraph;

class RenderGraphBuilder
{
public:
    RenderGraphBuilder( RenderGraph& g, int passID )
        : m_graph( g )
        , m_passIndex( passID ) {
    }

    std::string create_target( const std::string& name, const TargetInfo& info );
    std::string read( const std::string& name, const ReadInfo& info, ImageLayout expectedLayout );
    void        write( const std::string& name );
    // std::string create_swapchain_target( const std::string& name, const Graphics::RenderTargetInfo& info );

private:
    RenderGraph& m_graph;
    int          m_passIndex;
};

struct RenderPassInfo {
    std::string                                                                                name;
    std::function<void( RenderGraphBuilder& )>                                                 setupCallback;
    std::function<void( const RenderView&, const RenderResources&, const RenderPassOutputs& )> executeCallback;
    std::vector<std::string>                                                                   shaderProgramKeys;

    std::vector<std::string> writeAttachmentsKeys;
    std::vector<std::string> readAttachmentKeys;

    struct RuntimeData {
        Graphics::RenderPass               renderPass;
        std::vector<Graphics::Framebuffer> fbos;
        bool                               validFBO        = false;
        bool                               validRenderPass = false;
    } runtime;
};

class RenderGraph
{
private:
    ptr<Graphics::Device> m_device;
    Render::Frame*        m_frame = nullptr;

    std::vector<RenderPassInfo>                                     m_passes;
    std::unordered_map<std::string, RenderGraphResource>            m_transientResources; // Graph Resource (image ID)
    std::unordered_map<std::string, RenderGraphAttachment>          m_attachmentCache;    // Image attachments (image ID)
    std::unordered_map<std::string, RenderPassInfo::RuntimeData>    m_renderCache;        // Render runtime data (pass ID)
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> m_shaderCache;        // Pre-registered shaders (shader ID)

    friend class RenderGraphBuilder;

    bool reconcile( const RenderGraphResource& res, const Graphics::Image& image ) {
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
    void bake() {

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
                if ( dirty || reconcileRead( res, attachment.textures[name] ) )
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

            // Compile and prepare shaders programs
            for ( const auto& shaderKey : pass.shaderProgramKeys )
            {
                auto& shader = m_shaderCache[shaderKey];
                if ( !shader->compiled() || !rt.validRenderPass )
                {
                    shader->cleanup();
                    shader->create_descriptor_layouts( m_device );

                    // shader.m_shaderpass = m_device->create_shader_pass();
                    shader->m_compiled = true;
                }

                // Update uniforms and allocate ALWAYS !!
                // Allocate
                for ( auto& bindingSet : shader->m_uniformBindings )
                {
                    // m_frame
                }
                // Update
            }

            rt.validRenderPass = true;
        }
    }

public:
    void
    begin_frame( Frame& f ) {
        m_frame = &f;
        m_passes.clear();
        m_transientResources.clear();
        m_frame->wait();
    }

    int add_pass( const std::string&                                                                         name,
                  const std::vector<std::string>&                                                            shaderPrograms,
                  std::function<void( RenderGraphBuilder& )>                                                 onSetup,
                  std::function<void( const RenderView&, const RenderResources&, const RenderPassOutputs& )> onExecute ) {
        int id = static_cast<int>( m_passes.size() );
        m_passes.push_back( { name, onSetup, onExecute, shaderPrograms } );
        RenderGraphBuilder builder( *this, id );
        onSetup( builder );
        return id;
    }

    void end_frame( const RenderView& view, const RenderResources& shared ) {
        // Baking
        bake();
        // Begin Command Buffer
        m_frame->start();
        // Execute Passes
        for ( auto& p : m_passes )
        {
            // RenderPassOutputs outputs = build_outputs( p.name );
            // auto&             rt      = runtimeCache[p.name];
            // p.execute( view, shared, outputs );
        }
        // End Command Buffer
        m_frame->end();
        // Submit worload
        m_frame->submit();
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
        return m_transientResources[name];
    }

    template <typename T, typename... Args>
    void register_shader( const std::string& name, Args&&... args ) {
        m_shaderPrograms[name] = std::make_unique<T>( name, std::forward<Args>( args )... );
    }
    // register_shader<GraphicShaderProgram>( "lighting", "shaders/lighting.glsl", uniformBindings, settings );
};

std::string RenderGraphBuilder::create_target( const std::string& name, const TargetInfo& info ) {
    auto& r       = m_graph.get_or_create_resource( name );
    r.info        = info;
    r.written     = true;
    r.firstWriter = m_passIndex;

    m_graph.m_passes[m_passIndex].writeAttachmentsKeys.push_back( name );

    return name;
}

std::string RenderGraphBuilder::read( const std::string& name, const ReadInfo& info, ImageLayout expectedLayout ) {
    m_graph.get_or_create_resource( name );

    m_graph.m_passes[m_passIndex].readAttachmentKeys.push_back( name );

    return name;
}

void RenderGraphBuilder::write( const std::string& name ) {
    auto& r   = m_graph.get_or_create_resource( name );
    r.written = true;
    // r.firstWriter = m_passIndex;

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
