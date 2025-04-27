/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VOXELIZATION_PASS_H
#define VOXELIZATION_PASS_H
#include <engine/core/passes/graphic_pass.h>
#include <engine/core/resource_manager.h>

#define USE_IMG_ATOMIC_OPERATION

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
Performs an voxelization of the direct irradiance of the scene. For setting the edges of the volume it takes into
account the AABB of the scene.
*/
class VoxelizationPass final : public BaseGraphicPass
{
#ifdef USE_IMG_ATOMIC_OPERATION
    const uint16_t RESOURCE_IMAGES = 4;
#else
    const uint16_t RESOURCE_IMAGES = 1;
#endif

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    void create_voxelization_image();
    void setup_material_descriptor(IMaterial* mat);

  public:
    /*

                 Input Attachments:
                 -
                 - Shadow Map

                 Output Attachments:
                 -
                 - VoxeiLzed Irradiance (Texture 3D)

             */
    VoxelizationPass(Graphics::Device* device, const PassLinkage<1, 1>& config, uint32_t resolution)
        : BaseGraphicPass(device, {resolution, resolution}, 1, 1, false, false, "VOXELIZATION") {
        BasePass::store_attachments<1, 1>(config);
    }

    void create_framebuffer() override;

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                               std::vector<Graphics::SubPassDependency>& dependencies) override;

    void setup_uniforms(std::vector<Graphics::Frame>& frames) override;

    void setup_shader_passes() override;

    void link_input_attachments() override;

    void update_uniforms(uint32_t frameIndex, Scene* const scene) override;

    void resize_attachments() override;

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0) override;

    void cleanup() override;
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif