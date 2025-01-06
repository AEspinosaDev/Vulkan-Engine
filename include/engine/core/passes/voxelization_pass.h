/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VOXELIZATION_PASS_H
#define VOXELIZATION_PASS_H
#include <engine/core/passes/pass.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class VoxelizationPass : public GraphicPass
{
    Graphics::Image m_voxelization = {};

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    void create_voxelization_image();
    void setup_material_descriptor(IMaterial* mat);

  public:
    VoxelizationPass(Graphics::Device* ctx, uint32_t resolution)
        : BasePass(ctx, {resolution, resolution}, 1, 1, false, "VOXELIZATION") {
    }

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void cleanup();
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif