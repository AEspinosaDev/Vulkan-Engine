// #ifndef RENDER_GRAPH
// #define RENDER_GRAPH

// #include <engine/common.h>
// #include <engine/render/passes/pass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN
// namespace Render {

// // Forward declarations
// struct FrameContext;
// struct RenderView;
// class RenderResources;

// //----------------------------------------------
// // Render Targets
// //----------------------------------------------

// // Defines the attachment (image) for the framebuffer renderpass
// struct RenderTargetInfo {
//     FormatType      format;
//     ImageUsageFlags usage;
//     ImageLayout     layout;
//     bool            load    = false; // If not load is cleaned (stencil + color)
//     bool            store   = true;
//     bool            isDepth = false; // This can be inferenced from the color format !!!
//     ClearValue      clearValue{};
// };

// // It should be an image
// struct RuntimeResource {
//     VkImage        image  = VK_NULL_HANDLE;
//     VkImageView    view   = VK_NULL_HANDLE;
//     VkDeviceMemory memory = VK_NULL_HANDLE;
//     bool           valid  = false;
// };

// // Resource data
// class RenderGraphResource
// {
//   public:
//     std::string      name;
//     RenderTargetInfo info;
//     bool             written     = false;
//     int              firstWriter = -1;
// };

// struct RenderPassOutputs {
//     std::unordered_map<std::string, VkImageView> views;
//     VkFramebuffer                                framebuffer = VK_NULL_HANDLE;

//     VkImageView get(const std::string& name) const {
//         auto it = views.find(name);
//         return it != views.end() ? it->second : VK_NULL_HANDLE;
//     }
// };

// class ShaderProgram
// {
//   public:
//     std::string      name;
//     VkPipeline       pipeline = VK_NULL_HANDLE;
//     VkPipelineLayout layout   = VK_NULL_HANDLE;

//     ShaderProgram(const std::string& name)
//         : name(name) {
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

// class RenderGraph;

// class RenderGraphBuilder
// {
//   public:
//     RenderGraphBuilder(RenderGraph& g, int passID)
//         : m_graph(g)
//         , m_passIndex(passID) {
//     }

//     std::string create_target(const std::string& name, const RenderTargetInfo& info);
//     std::string read(const std::string& name);
//     void        write(const std::string& name);

//   private:
//     RenderGraph& m_graph;
//     int          m_passIndex;
// };

// struct RenderPassInfo {
//     std::string                                                                              name;
//     std::function<void(RenderGraphBuilder&)>                                                 setup;
//     std::function<void(const RenderView&, const RenderResources&, const RenderPassOutputs&)> execute;

//     struct RuntimeData {
//         VkRenderPass     renderPass     = VK_NULL_HANDLE;
//         VkFramebuffer    framebuffer    = VK_NULL_HANDLE;
//         ShaderProgram*   program        = nullptr;
//         VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
//         VkDescriptorSet  descriptorSet  = VK_NULL_HANDLE;
//         bool             created        = false;
//     } runtime;
// };

// class RenderGraph
// {
//   private:
//     FrameContext const*         frame = nullptr;
//     std::vector<RenderPassInfo> passes;

//     std::unordered_map<std::string, RenderGraphResource>         transientResources;
//     std::unordered_map<std::string, RuntimeResource>             persistentResources;
//     std::unordered_map<std::string, ShaderProgram>               shaderCache;
//     std::unordered_map<std::string, RenderPassInfo::RuntimeData> runtimeCache;

//     void create_image_and_view(const RenderTargetInfo& info, RuntimeResource& out) {
//         out.valid = true;
//     }

//     void create_renderpass(RenderPassInfo::RuntimeData& rt) {
//         // TODO
//     }

//     void create_framebuffer(RenderPassInfo::RuntimeData& rt) {
//         // TODO
//     }

//     void create_pipeline(RenderPassInfo::RuntimeData& rt) {
//         if (rt.program && rt.program->pipeline == VK_NULL_HANDLE)
//         {
//             ShaderProgram* program = rt.program;
//             program->pipeline      = VK_NULL_HANDLE;
//             program->layout        = VK_NULL_HANDLE;
//         }
//     }

//     void create_descriptor_pool_and_set(RenderPassInfo::RuntimeData& rt) {
//         // ShaderProgram* program = rt.program;
//         // if (!program || program->layout == VK_NULL_HANDLE) return;

//         // std::vector<VkDescriptorPoolSize> poolSizes = {
//         //     { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8 },
//         //     { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4 },
//         // };

//         // VkDescriptorPoolCreateInfo poolInfo{
//         //     .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
//         //     .maxSets = 1,
//         //     .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
//         //     .pPoolSizes = poolSizes.data(),
//         // };
//         // vkCreateDescriptorPool(device, &poolInfo, nullptr, &rt.descriptorPool);

//         // VkDescriptorSetAllocateInfo allocInfo{
//         //     .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//         //     .descriptorPool = rt.descriptorPool,
//         //     .descriptorSetCount = 1,
//         //     .pSetLayouts = &program->layout,
//         // };
//         // vkAllocateDescriptorSets(device, &allocInfo, &rt.descriptorSet);
//     }

//     bool resolution_changed(const RenderTargetInfo& info, const RuntimeResource& runtime) {
//         return false;
//     }

//     void compile() {
//         for (auto& [name, res] : transientResources)
//         {
//             auto&       runtime = persistentResources[name];
//             const auto& info    = res.info;
//             if (!runtime.valid || resolution_changed(info, runtime))
//             {
//                 create_image_and_view(info, runtime);
//             }
//         }

//         for (auto& pass : passes)
//         {
//             auto& rt = runtimeCache[pass.name];
//             if (!rt.created)
//             {
//                 rt.program = pass.program;
//                 create_renderpass(rt);
//                 create_framebuffer(rt);
//                 create_pipeline(rt);
//                 create_descriptor_pool_and_set(rt);
//                 rt.created = true;
//             }
//         }
//     }

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

//   public:
//     void begin_frame(const FrameContext& f) {
//         frame = &f;
//         passes.clear();
//         transientResources.clear();
//     }

//     int add_pass(const std::string&                                                                       name,
//                  ShaderProgram*                                                                           shader,
//                  std::function<void(RenderGraphBuilder&)>                                                 setup,
//                  std::function<void(const RenderView&, const RenderResources&, const RenderPassOutputs&)> exec) {
//         int id = static_cast<int>(passes.size());
//         passes.push_back({name, setup, exec, shader});
//         RenderGraphBuilder builder(*this, id);
//         setup(builder);
//         return id;
//     }

//     void end_frame(const RenderView& view, const RenderResources& shared) {
//         compile();
//         for (auto& p : passes)
//         {
//             RenderPassOutputs outputs = build_outputs(p.name);
//             auto&             rt      = runtimeCache[p.name];
//             p.execute(view, shared, outputs);
//         }
//     }

//     RenderGraphResource& get_or_create_resource(const std::string& name) {
//         if (!transientResources.count(name))
//         {
//             transientResources[name] = RenderGraphResource{.name = name};
//             if (!persistentResources.count(name))
//             {
//                 persistentResources[name] = {};
//             }
//         }
//         return transientResources[name];
//     }

//     RuntimeResource& get_runtime_resource(const std::string& name) {
//         return persistentResources.at(name);
//     }

//     ShaderProgram* get_shader_program(const std::string& name) {
//         return shaderCache.count(name) ? &shaderCache[name] : nullptr;
//     }

//     void register_shader(ShaderProgram&& shader) {
//         shaderCache[shader.name] = std::move(shader);
//     }
// };

// std::string RenderGraphBuilder::create_target(const std::string& name, const RenderTargetInfo& info) {
//     auto& r       = m_graph.get_or_create_resource(name);
//     r.info        = info;
//     r.written     = true;
//     r.firstWriter = m_passIndex;
//     return name;
// }

// std::string RenderGraphBuilder::read(const std::string& name) {
//     m_graph.get_or_create_resource(name);
//     return name;
// }

// void RenderGraphBuilder::write(const std::string& name) {
//     auto& r       = m_graph.get_or_create_resource(name);
//     r.written     = true;
//     r.firstWriter = m_passIndex;
// }

// // class GBufferPass : public IRenderPass {
// // public:
// //     ShaderProgram* shader = nullptr;

// //     GBufferPass(ShaderProgram* program) : shader(program) {}

// //     void build(RenderGraphBuilder& builder) override {
// //         builder.create_target("depth", RenderTargetInfo{
// //             .format = VK_FORMAT_D32_SFLOAT,
// //             .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
// //             .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
// //             .clearValue = {.depthStencil = {1.0f, 0}},
// //             .isDepth = true
// //         });

// //         builder.create_target("albedo", RenderTargetInfo{
// //             .format = VK_FORMAT_R8G8B8A8_UNORM,
// //             .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
// //             .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
// //             .clearValue = {.color = {{0, 0, 0, 1}}}
// //         });
// //     }

// //     void execute(const RenderView& view, const FrameContext& frame, const RenderResources& shared, const RenderPassOutputs& outputs, ShaderProgram* program)
// //     override {
// //         // Bind pipeline, descriptor set, draw meshes
// //     }
// // };

// // void exampleUsage(RenderGraph& graph, const RenderView& view, const FrameContext& frame, const RenderResources& shared) {
// //     ShaderProgram gbufferShader("GBuffer");
// //     ShaderProgram lightingShader("Lighting");

// //     graph.register_shader(std::move(gbufferShader));
// //     graph.register_shader(std::move(lightingShader));

// //     graph.begin_frame(frame);

// //     GBufferPass gpass(graph.get_shader_program("GBuffer"));
// //     graph.add_pass("GBuffer", gpass.shader, [&](RenderGraphBuilder& b) { gpass.build(b); },
// //                    [&](const RenderView& view, const RenderResources& shared, const RenderPassOutputs& outputs) {
// //                        gpass.execute(view, frame, shared, outputs, gpass.shader);
// //                    });

// //     graph.add_pass("Lighting", graph.get_shader_program("Lighting"),
// //         [&](RenderGraphBuilder& builder) {
// //             builder.read("depth");
// //             builder.read("albedo");
// //             builder.create_target("lighting", RenderTargetInfo{
// //                 .format = VK_FORMAT_R16G16B16A16_SFLOAT,
// //                 .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
// //                 .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
// //                 .clearValue = {.color = {{0, 0, 0, 1}}},
// //                 .load = false,
// //                 .store = true
// //             });
// //         },
// //         [&](const RenderView& view, const RenderResources& shared, const RenderPassOutputs& outputs) {
// //             auto* program = graph.get_shader_program("Lighting");
// //             // Bind pipeline and descriptor set, draw fullscreen quad
// //         });

// //     graph.end_frame(view, shared);
// // }

// } // namespace Render
// VULKAN_ENGINE_NAMESPACE_END

// #endif
