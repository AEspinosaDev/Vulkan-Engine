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
Procedural Physically Based Sky generation pass.
*/
class SkyPass : public GraphicPass
{
  protected:
    Mesh* m_vignette;
    Graphics::DescriptorSet m_LUTdescriptor;

  public:
    SkyPass(Graphics::Device* ctx, Extent2D extent, Mesh* vignette)
        : BasePass(ctx, extent, 2, 1, false, "SKY GENERATION")
        , m_vignette(vignette) {
    }

    virtual void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                   std::vector<Graphics::SubPassDependency>& dependencies);

    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames);

    virtual void setup_shader_passes();

    virtual void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif