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

// class IRenderPass
// {
//   public:
//     virtual ~IRenderPass()                          = default;
//     virtual void build(RenderGraphBuilder& builder) = 0;
//     virtual void
//     execute(const RenderView& view, const FrameContext& frame, const RenderResources& shared, const RenderPassOutputs& outputs, ShaderProgram* program) = 0;
// };