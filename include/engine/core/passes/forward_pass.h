/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FORWARD_PASS_H
#define FORWARD_PASS_H
#include <engine/core/passes/pass.h>
#include <engine/core/textures/texture.h>
#include <engine/core/textures/textureLDR.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
STANDARD FORWARD LIGHTING PASS
*/
class ForwardPass : public GraphicPass
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    ColorFormatType m_depthFormat;
    MSAASamples     m_aa;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    void setup_material_descriptor(IMaterial* mat);

  public:
    ForwardPass(Graphics::Device* ctx,
                Extent2D          extent,
                uint32_t          framebufferCount,
                ColorFormatType   colorFormat,
                ColorFormatType   depthFormat,
                MSAASamples       samples,
                bool              isDefault = true)
        : BasePass(ctx, extent, framebufferCount, 1, isDefault, "FORWARD")
        , m_colorFormat(colorFormat)
        , m_depthFormat(depthFormat)
        , m_aa(samples) {
    }

    void setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void connect_to_previous_images(std::vector<Graphics::Image> images);

    void set_envmap_descriptor(Graphics::Image env, Graphics::Image irr);
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif