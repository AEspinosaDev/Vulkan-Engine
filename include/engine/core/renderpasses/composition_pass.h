/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMPOSITION_PASS_H
#define COMPOSITION_PASS_H
#include <engine/core/renderpasses/renderpass.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {

/*
DEFERRED RENDERING LIGHTING PASS
*/
class CompositionPass : public RenderPass
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    Mesh*           m_vignette;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet gBufferDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

  public:
    CompositionPass(Graphics::Device* ctx,
                    VkExtent2D        extent,
                    uint32_t          framebufferCount,
                    ColorFormatType   colorFormat,
                    Mesh*             vignette,
                    bool              isDefault = true)
        : RenderPass(ctx, extent, framebufferCount, 1, isDefault)
        , m_colorFormat(colorFormat)
        , m_vignette(vignette) {
    }

    void setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void connect_to_previous_images(std::vector<Graphics::Image> images);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void set_envmap_descriptor(Graphics::Image env, Graphics::Image irr);

    
   
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif