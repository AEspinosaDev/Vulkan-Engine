/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GUI_PASS_H
#define GUI_PASS_H

#include <engine/core/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {

/*
GUI Overlay Pass. It does not need linkage information because it takes the swapchain image as input and output
attachment. As it is designed to be at the last step of the pipeline.

This pass takes the default pass image (swapchain image)
*/
class GUIPass : public BaseGraphicPass
{

  public:
    GUIPass(Graphics::Device* device, VkExtent2D extent)
        : BaseGraphicPass(device, extent, 1, 1, true, true, "GUI") {
    }

    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames) override {
    }
    virtual void setup_shader_passes() override {
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                               std::vector<Graphics::SubPassDependency>& dependencies) override;

    void clean_framebuffer() override;

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0) override;
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif