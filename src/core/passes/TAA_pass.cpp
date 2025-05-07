#include <engine/core/passes/TAA_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {
void TAAPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) {
    attachments.resize(1);

    attachments[0] =
        Graphics::AttachmentConfig(m_colorFormat,
                                   1,
                                   LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   this->m_isDefault ? IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT
                                                     : IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                   COLOR_ATTACHMENT,
                                   ASPECT_COLOR,
                                   TEXTURE_2D,
                                   FILTER_LINEAR,
                                   ADDRESS_MODE_CLAMP_TO_EDGE);

    attachments[0].isDefault = this->m_isDefault ? true : false;

    // Depdencies
    if (!this->m_isDefault)
    {
        dependencies.resize(2);

        dependencies[0]                 = Graphics::SubPassDependency(STAGE_FRAGMENT_SHADER, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
        dependencies[0].srcAccessMask   = ACCESS_SHADER_READ;
        dependencies[0].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
        dependencies[1]                 = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_FRAGMENT_SHADER, ACCESS_SHADER_READ);
        dependencies[1].srcAccessMask   = ACCESS_COLOR_ATTACHMENT_WRITE;
        dependencies[1].srcSubpass      = 0;
        dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[1].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
    } else
    {
        dependencies.resize(1);

        // dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[0] = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
        dependencies[0].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
    }
}
void TAAPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    // Init and configure local descriptors
    this->m_descriptorPool = this->m_device->create_descriptor_pool(1, 3, 1, 1, 1);

    std::vector<VKFW::Graphics::LayoutBinding> bindings;
    for (size_t i = 0; i < 3; i++)
    {
        bindings.push_back(Graphics::LayoutBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, i));
    }
    this->m_descriptorPool.set_layout(GLOBAL_LAYOUT, bindings);

    this->m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &this->m_imageDescriptorSet);
}

void TAAPass::create_framebuffer() {
    // create prev frame image
    ImageConfig prevImgConfig = {};
    prevImgConfig.format      = m_colorFormat;
    prevImgConfig.usageFlags  = IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_DST | IMAGE_USAGE_TRANSFER_SRC;
    m_interAttachments[0]     = m_device->create_image({m_imageExtent.width, m_imageExtent.height, 1}, prevImgConfig);
    m_interAttachments[0].create_view(prevImgConfig);

    SamplerConfig samplerConfig      = {};
    samplerConfig.samplerAddressMode = ADDRESS_MODE_CLAMP_TO_EDGE;
    m_interAttachments[0].create_sampler(samplerConfig);

    PostProcessPass::create_framebuffer();
}

void TAAPass::link_input_attachments() {
    PostProcessPass::link_input_attachments();
    this->m_imageDescriptorSet.update(&m_interAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
}

void TAAPass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    auto cmd = currentFrame.commandBuffer;

    /*Prepare previous image for reading in case is recreated*/
    if (m_interAttachments[0].currentLayout == LAYOUT_UNDEFINED)
        cmd.pipeline_barrier(m_interAttachments[0],
                             LAYOUT_UNDEFINED,
                             LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                             ACCESS_NONE,
                             ACCESS_SHADER_READ,
                             STAGE_TOP_OF_PIPE,
                             STAGE_FRAGMENT_SHADER);

    /*RESOLVE FINAL IMAGE*/
    PostProcessPass::execute(currentFrame, scene, presentImageIndex);

    /////////////////////////////////////////
    /*Copy data to tmp previous frame image*/
    /////////////////////////////////////////
    cmd.pipeline_barrier(m_interAttachments[0],
                         LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         LAYOUT_TRANSFER_DST_OPTIMAL,
                         ACCESS_SHADER_READ,
                         ACCESS_TRANSFER_READ,
                         STAGE_FRAGMENT_SHADER,
                         STAGE_TRANSFER);

    cmd.blit_image(*m_framebuffers[m_isDefault ? presentImageIndex : 0].attachmentImagesPtrs[0], m_interAttachments[0], FILTER_NEAREST);

    cmd.pipeline_barrier(*m_framebuffers[m_isDefault ? presentImageIndex : 0].attachmentImagesPtrs[0],
                         LAYOUT_TRANSFER_SRC_OPTIMAL,
                         m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         ACCESS_TRANSFER_READ,
                         ACCESS_SHADER_READ,
                         STAGE_TRANSFER,
                         STAGE_FRAGMENT_SHADER);

    cmd.pipeline_barrier(m_interAttachments[0],
                         LAYOUT_TRANSFER_DST_OPTIMAL,
                         LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         ACCESS_TRANSFER_WRITE,
                         ACCESS_SHADER_READ,
                         STAGE_TRANSFER,
                         STAGE_FRAGMENT_SHADER);
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END