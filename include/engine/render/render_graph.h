// // Hybrid Vulkan-like Render Graph with support for lambdas and class-based passes

// #include <vulkan/vulkan.h>
// #include <string>
// #include <unordered_map>
// #include <vector>
// #include <functional>
// #include <memory>
// #include <cassert>

// // Forward declarations
// struct FrameContext;
// struct RenderView;
// class RenderResources;

// //----------------------------------------------
// // Render Targets
// //----------------------------------------------

// struct RenderTargetInfo {
//     VkFormat format;
//     VkImageUsageFlags usage;
//     VkImageLayout layout;
//     VkClearValue clearValue{};
//     bool load = false;
//     bool store = true;
//     bool isDepth = false;
// };

// class RenderGraphResource {
// public:
//     std::string name;
//     RenderTargetInfo info;
//     bool written = false;
//     int firstWriter = -1;
// };

// struct RenderPassOutputs {
//     std::unordered_map<std::string, VkImageView> views;
//     VkFramebuffer framebuffer;

//     VkImageView get(const std::string& name) const {
//         auto it = views.find(name);
//         return it != views.end() ? it->second : VK_NULL_HANDLE;
//     }
// };

// //----------------------------------------------
// // Base RenderPass Class
// //----------------------------------------------

// class IRenderPass {
// public:
//     virtual ~IRenderPass() {}
//     virtual void build(class RenderGraphBuilder&) = 0;
//     virtual void execute(const RenderView&, const FrameContext&, const RenderResources&, const RenderPassOutputs&) = 0;
// };

// //----------------------------------------------
// // Shader Program abstraction (mocked)
// //----------------------------------------------

// class ShaderProgram {
// public:
//     ShaderProgram(const std::string& name) : id(name) {}
//     void bind(VkCommandBuffer) {} // mock
//     void setUniform(const std::string& name, void* data, size_t size) {} // mock
//     std::string id;
// };

// //----------------------------------------------
// // RenderGraph Builder
// //----------------------------------------------

// class RenderGraph;

// class RenderGraphBuilder {
// public:
//     RenderGraphBuilder(RenderGraph& g, int passID) : graph(g), passIndex(passID) {}

//     std::string createTarget(const std::string& name, const RenderTargetInfo& info);
//     std::string read(const std::string& name);
//     void write(const std::string& name);

// private:
//     RenderGraph& graph;
//     int passIndex;
// };

// //----------------------------------------------
// // RenderGraph
// //----------------------------------------------

// struct FrameContext {};
// struct RenderView {};
// class RenderResources {};

// struct RenderPassInfo {
//     std::string name;
//     std::function<void(RenderGraphBuilder&)> setup;
//     std::function<void(const RenderView&, const FrameContext&, const RenderResources&, const RenderPassOutputs&)> execute;
// };

// class RenderGraph {
// public:
//     void beginFrame(const FrameContext& f) {
//         frame = &f;
//         passes.clear();
//         resources.clear();
//     }

//     int addPass(const std::string& name, IRenderPass* pass) {
//         int id = static_cast<int>(passes.size());
//         auto builder = RenderGraphBuilder(*this, id);
//         pass->build(builder);

//         passes.push_back(RenderPassInfo{
//             .name = name,
//             .setup = {},
//             .execute = [pass](const RenderView& view, const FrameContext& f, const RenderResources& shared, const RenderPassOutputs& outputs) {
//                 pass->execute(view, f, shared, outputs);
//             }
//         });
//         return id;
//     }

//     int addPass(const std::string& name,
//                 std::function<void(RenderGraphBuilder&)> setup,
//                 std::function<void(const RenderView&, const FrameContext&, const RenderResources&, const RenderPassOutputs&)> exec) {
//         int id = static_cast<int>(passes.size());
//         passes.push_back({name, setup, exec});
//         RenderGraphBuilder builder(*this, id);
//         setup(builder);
//         return id;
//     }

//     void endFrame(const RenderView& view, const RenderResources& shared) {
//         compile();
//         for (auto& p : passes) {
//             RenderPassOutputs outputs = buildOutputs(p.name);
//             p.execute(view, *frame, shared, outputs);
//         }
//     }

//     RenderGraphResource& getOrCreateResource(const std::string& name) {
//         if (!resources.count(name)) {
//             resources[name] = RenderGraphResource{.name = name};
//         }
//         return resources[name];
//     }

// private:
//     FrameContext const* frame;
//     std::vector<RenderPassInfo> passes;
//     std::unordered_map<std::string, RenderGraphResource> resources;

//     void compile() {
//         // Allocate Vulkan images, views, renderpasses, barriers, etc.
//     }

//     RenderPassOutputs buildOutputs(const std::string& passName) {
//         return {}; // mock
//     }
// };

// // Builder method definitions
// std::string RenderGraphBuilder::createTarget(const std::string& name, const RenderTargetInfo& info) {
//     auto& r = graph.getOrCreateResource(name);
//     r.info = info;
//     r.written = true;
//     r.firstWriter = passIndex;
//     return name;
// }

// std::string RenderGraphBuilder::read(const std::string& name) {
//     graph.getOrCreateResource(name);
//     return name;
// }

// void RenderGraphBuilder::write(const std::string& name) {
//     auto& r = graph.getOrCreateResource(name);
//     r.written = true;
//     r.firstWriter = passIndex;
// }

// //----------------------------------------------
// // Example GBuffer Pass using Class
// //----------------------------------------------

// class GBufferPass : public IRenderPass {
// public:
//     ShaderProgram shader;

//     GBufferPass() : shader("gbuffer") {}

//     void build(RenderGraphBuilder& builder) override {
//         builder.createTarget("depth", RenderTargetInfo{
//             .format = VK_FORMAT_D32_SFLOAT,
//             .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//             .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
//             .clearValue = {.depthStencil = {1.0f, 0}},
//             .isDepth = true
//         });

//         builder.createTarget("albedo", RenderTargetInfo{
//             .format = VK_FORMAT_R8G8B8A8_UNORM,
//             .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//             .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//             .clearValue = {.color = {{0, 0, 0, 1}}}
//         });
//     }

//     void execute(const RenderView& view, const FrameContext& frame, const RenderResources& resources, const RenderPassOutputs& outputs) override {
//         shader.bind(VK_NULL_HANDLE); // mock bind
//         shader.setUniform("GlobalUBO", nullptr, 0); // mock
//         // Draw meshes...
//     }
// };

// //----------------------------------------------
// // Usage in render loop
// //----------------------------------------------

// void render(RenderGraph& graph, const RenderView& view, const FrameContext& frame, const RenderResources& shared) {
//     graph.beginFrame(frame);

//     static GBufferPass gbuffer;
//     graph.addPass("GBuffer", &gbuffer);

//     graph.addPass("Lighting",
//         [&](RenderGraphBuilder& builder) {
//             builder.read("depth");
//             builder.read("albedo");
//             builder.createTarget("lighting", RenderTargetInfo{
//                 .format = VK_FORMAT_R16G16B16A16_SFLOAT,
//                 .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//                 .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//                 .clearValue = {.color = {{0, 0, 0, 1}}}
//             });
//         },
//         [&](const RenderView& view, const FrameContext& frame, const RenderResources& shared, const RenderPassOutputs& out) {
//             // lighting pass execution
//         });

//     graph.endFrame(view, shared);
// }
