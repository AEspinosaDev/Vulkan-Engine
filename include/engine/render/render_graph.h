#ifndef RENDER_GRAPH
#define RENDER_GRAPH

#include <engine/common.h>
#include <engine/graphics/device.h>
#include <engine/graphics/renderpass.h>
#include <engine/graphics/texture.h>
#include <engine/render/frame.h>
#include <engine/render/program/shader_program.h>
#include <engine/render/render_resources.h>
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

    std::unordered_map<std::string, ReadInfo> readInfos; // pass name ID

    struct LayoutUse {
        std::string          passName;
        VkImageLayout        layout;
        VkAccessFlags        access;
        VkPipelineStageFlags stage;
        bool                 isWrite = false;
    };
    std::vector<LayoutUse> layoutHistory;
};

class RenderGraphAttachment
{
public:
    Graphics::Image                                    image = {};
    std::unordered_map<std::string, Graphics::Texture> textures;     // pass name ID
    bool                                               used = false; // if needs to be destroyed
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
    std::string create_swapchain_target( const std::string& name );
    std::string read( const std::string& name, const ReadInfo& info );
    void        write( const std::string& name );

private:
    RenderGraph& m_graph;
    int          m_passIndex;
};

class RenderGraphReconciler
{
public:
    RenderGraphReconciler( RenderGraph& g )
        : m_graph( g ) {
    }
    void reconcile_attachments();
    void reconcile_renderpasses();
    void reconcile_shaders();

private:
    RenderGraph& m_graph;
};

struct RenderPassOutputs {
    Graphics::RenderPass&               renderPass;
    std::vector<Graphics::Framebuffer>& fbos;
};

struct RenderPassInfo {
    std::string                                                                          name;
    std::function<void( RenderGraphBuilder& )>                                           setupCallback;
    std::function<void( const RenderView&, const Resources&, const RenderPassOutputs& )> executeCallback;
    std::vector<std::string>                                                             shaderProgramKeys;

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
    std::unordered_map<std::string, RenderGraphResource>            m_transientResources; // Graph Resource         (image ID)
    std::unordered_map<std::string, RenderGraphAttachment>          m_attachmentCache;    // Image attachments      (image ID)
    std::unordered_map<std::string, RenderPassInfo::RuntimeData>    m_renderCache;        // Render runtime data    (pass ID)
    std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaderCache;        // Pre-registered shaders (shader ID)

    friend class RenderGraphBuilder;
    friend class RenderGraphReconciler;

    bool reconcile( const RenderGraphResource& res, const Graphics::Image& image );
    bool reconcileRead( const ReadInfo& info, const Graphics::Texture& texture );
    void bake();
    void transition_attachments( const RenderPassInfo& pass );

public:
    void begin_frame( Frame& f );
    int  add_pass( const std::string&                                                                   name,
                   const std::vector<std::string>&                                                      shaderPrograms,
                   std::function<void( RenderGraphBuilder& )>                                           onSetup,
                   std::function<void( const RenderView&, const Resources&, const RenderPassOutputs& )> onExecute );
    void end_frame( const RenderView& view, const Resources& shared );

    RenderGraphResource& get_or_create_resource( const std::string& name );

    template <typename T, typename... Args>
    void register_shader( const std::string& name, Args&&... args ) {
        m_shaderCache[name] = std::make_unique<T>( name, std::forward<Args>( args )... );
    }
    // register_shader<GraphicShaderProgram>( "lighting", "shaders/lighting.glsl", uniformBindings, settings );
};

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
