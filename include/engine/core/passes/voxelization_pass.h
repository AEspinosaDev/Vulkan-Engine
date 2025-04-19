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
class VoxelizationPass : public GraphicPass
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
    VoxelizationPass(Graphics::Device* ctx, uint32_t resolution)
        : GraphicPass(ctx, {resolution, resolution}, 1, 1, false, "VOXELIZATION") {
        m_resourceImages.resize(RESOURCE_IMAGES);
    }

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void link_previous_images(std::vector<Graphics::Image> images);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void resize_attachments();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void cleanup();
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif