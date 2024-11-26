/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GEOMETRY_PASS_H
#define GEOMETRY_PASS_H
#include <engine/core/renderpasses/renderpass.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class GeometryPass : public RenderPass
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    ColorFormatType m_depthFormat;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    void setup_material_descriptor(IMaterial* mat);

  public:
    GeometryPass(Graphics::Device* ctx,
                 Extent2D          extent,
                 uint32_t          framebufferCount,
                 ColorFormatType   colorFormat,
                 ColorFormatType   depthFormat,
                 bool              isDefault = true)
        : RenderPass(ctx, extent, framebufferCount, 1, isDefault)
        , m_colorFormat(colorFormat)
        , m_depthFormat(depthFormat) {
    }

    void setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif