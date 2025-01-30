/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SKY_PASS_H
#define SKY_PASS_H
#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {
/*
Procedural Physically Based Sky generation pass. Based on "A Scalable and Production Ready Sky and Atmosphere Rendering Technique"
by Sébastien Hillaire (2020).

Transmittance LUT is based on "Precomputed Atmospheric Scattering" by Eric Bruneton and Fabrice Neyret (2008).

Implementation by Fernando García Liñán 2023
*/
class SkyPass : public GraphicPass
{
  protected:
    Graphics::DescriptorSet m_imageDescriptor;

  public:
    SkyPass(Graphics::Device* ctx, Extent2D extent)
        : BasePass(ctx, extent, 3, 1, false, "SKY GENERATION") {
    }

    void create_framebuffer();

    virtual void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                   std::vector<Graphics::SubPassDependency>& dependencies);

    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames);

    virtual void setup_shader_passes();

    virtual void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif