/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GEOMETRY_PASS_H
#define GEOMETRY_PASS_H
#include <engine/core/passes/graphic_pass.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
DEFERRED RENDERING GEOMETRY PASS
*/
class GeometryPass : public GraphicPass
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
                 ColorFormatType   colorFormat,
                 ColorFormatType   depthFormat,
                 bool              isDefault = false)
        : GraphicPass(ctx, extent, 1, 1, isDefault, "GEOMETRY")
        , m_colorFormat(colorFormat)
        , m_depthFormat(depthFormat) {
    }

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void link_previous_images(std::vector<Graphics::Image> images);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif