// /*
//     This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

//     MIT License

//     Copyright (c) 2023 Antonio Espinosa Garcia

// */
// #ifndef SS_PASS_H
// #define SS_PASS_H
// #include <engine/core/passes/graphic_pass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN

// namespace Core {
// /*
// WIP
// */
// class SubSurfacePass : public BaseGraphicPass<2, 2>
// {
//   protected:
//     ColorFormatType         m_colorFormat;
//     Graphics::DescriptorSet m_imageDescriptorSet;

//   public:
//   SubSurfacePass(Graphics::Device* device, const PassConfig<0, 1>& config, Extent2D extent, ColorFormatType colorFormat)
//         : BaseGraphicPass(device, config, extent, 0, 0, false, "SS PASS")
//         , m_colorFormat(colorFormat) {
//     }

//     void create_framebuffer();

//     virtual void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
//                                        std::vector<Graphics::SubPassDependency>& dependencies);

//     virtual void setup_uniforms(std::vector<Graphics::Frame>& frames);

//     virtual void setup_shader_passes();

//     virtual void resize_attachments();

//     virtual void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

  
// };

// } // namespace Core
// VULKAN_ENGINE_NAMESPACE_END

// #endif