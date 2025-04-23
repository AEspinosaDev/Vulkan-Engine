// /*
//     This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

//     MIT License

//     Copyright (c) 2023 Antonio Espinosa Garcia

// */
// #ifndef SKINSSS_PASS_H
// #define SKINSSS_PASS_H
// #include <engine/core/passes/pass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN

// namespace Core {
// /*
// WIP
// */
// class SkinPass : public ComputePass
// {
//   protected:
//     Graphics::DescriptorSet m_imageDescriptorSet;

//   public:
//     PostProcessPass(Graphics::Device* ctx,
//                     Extent2D          extent)
//         : BasePass(ctx, extent, 0, 0, false, "SKIN PASS")
//         , m_colorFormat(colorFormat)
//         , m_shaderPath(shaderPath) {
//     }

//     virtual void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
//                                    std::vector<Graphics::SubPassDependency>& dependencies);

//     virtual void setup_uniforms(std::vector<Graphics::Frame>& frames);

//     virtual void setup_shader_passes();

//     virtual void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

//     virtual void link_input_attachments(std::vector<Graphics::Image> images);

//     void clean_framebuffer() {
//         GraphicPass::clean_framebuffer();
//     }
// };

// } // namespace Core
// VULKAN_ENGINE_NAMESPACE_END

// #endif