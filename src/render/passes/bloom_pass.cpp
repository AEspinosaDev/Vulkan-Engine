#include <engine/render/passes/bloom_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Render {

void BloomPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::AttachmentConfig(
        m_colorFormat,
        1,
        m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        m_isDefault ? IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST
                    : IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
        COLOR_ATTACHMENT,
        ASPECT_COLOR,
        TEXTURE_2D,
        FILTER_LINEAR,
        ADDRESS_MODE_CLAMP_TO_EDGE);

    attachments[0].isDefault = m_isDefault ? true : false;

    // Depdencies
    if (!m_isDefault)
    {
        dependencies.resize(2);

        dependencies[0]               = Graphics::SubPassDependency(STAGE_FRAGMENT_SHADER, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
        dependencies[0].srcAccessMask = ACCESS_SHADER_READ;
        dependencies[1]               = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_FRAGMENT_SHADER, ACCESS_SHADER_READ);
        dependencies[1].srcAccessMask = ACCESS_COLOR_ATTACHMENT_WRITE;
        dependencies[1].srcSubpass    = 0;
        dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
    } else
    {
        dependencies.resize(1);

        // dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[0] = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    }
}
void BloomPass::setup_uniforms(std::vector<Graphics::Frame>& frames, const Render::Resources& shared) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 4, 1, 1, 1, 1);

    LayoutBinding brightBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_COMPUTE, 0); // HDR input bright image
    LayoutBinding imageBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1); // HDR input color image

    LayoutBinding bloomMipsImgBinding(UNIFORM_STORAGE_IMAGE, SHADER_STAGE_COMPUTE, 2, MIPMAP_LEVELS); // Bloom image mipmaps
    LayoutBinding bloomMipsSamplBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_COMPUTE, 3, MIPMAP_LEVELS);

    LayoutBinding bloomBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4); // Resolved bloom image
                                                                                          // binding

    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {brightBinding, imageBinding, bloomMipsImgBinding, bloomMipsSamplBinding, bloomBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_imageDescriptorSet);
}
void BloomPass::setup_shader_passes() {

    const uint32_t MIPMAP_UNIFORM_SIZE   = sizeof(uint32_t) * 2.0f;
    const uint32_t SETTINGS_UNIFORM_SIZE = sizeof(float);

    ComputeShaderPass* downsamplePass               = new ComputeShaderPass(m_device->get_handle(), GET_RESOURCE_PATH("shaders/bloom/downsample.glsl"));
    downsamplePass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    downsamplePass->settings.pushConstants.push_back(PushConstant(SHADER_STAGE_COMPUTE, MIPMAP_UNIFORM_SIZE));

    downsamplePass->compile_shader_stages();
    downsamplePass->build(m_descriptorPool);

    m_shaderPasses["downsample"] = downsamplePass;

    ComputeShaderPass* upsamplePass               = new ComputeShaderPass(m_device->get_handle(), GET_RESOURCE_PATH("shaders/bloom/upsample.glsl"));
    upsamplePass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    upsamplePass->settings.pushConstants.push_back(PushConstant(SHADER_STAGE_COMPUTE, MIPMAP_UNIFORM_SIZE));

    upsamplePass->compile_shader_stages();
    upsamplePass->build(m_descriptorPool);

    m_shaderPasses["upsample"] = upsamplePass;

    GraphicShaderPass* bloomPass = new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/bloom/compose.glsl"));
    bloomPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    bloomPass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, false}, {COLOR_ATTRIBUTE, false}};
    bloomPass->settings.pushConstants.push_back(PushConstant(SHADER_STAGE_FRAGMENT, SETTINGS_UNIFORM_SIZE));

    bloomPass->compile_shader_stages();
    bloomPass->build(m_descriptorPool);

    m_shaderPasses["bloom"] = bloomPass;
}

void BloomPass::execute(const Render::RenderView& view, const Render::Resources& shared) {

    CommandBuffer cmd;

    if (m_bloomStrength != 0.0f)
    {

        cmd = view.commandBuffer;

        struct Mipmap {
            uint32_t srcLevel;
            uint32_t dstLevel;
        };

        const uint32_t WORK_GROUP_SIZE = 16;

        cmd.pipeline_barrier(*m_inAttachments[1],
                             LAYOUT_UNDEFINED,
                             LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                             ACCESS_SHADER_WRITE,
                             ACCESS_SHADER_READ,
                             STAGE_FRAGMENT_SHADER,
                             STAGE_COMPUTE_SHADER);
        cmd.pipeline_barrier(
            m_bloomImage, LAYOUT_UNDEFINED, LAYOUT_GENERAL, ACCESS_SHADER_WRITE, ACCESS_SHADER_READ, STAGE_COMPUTE_SHADER, STAGE_COMPUTE_SHADER);

        cmd.clear_image(m_bloomImage, LAYOUT_GENERAL);

        ////////////////////////////////////////////////////////////
        // DOWNSAMPLE
        ////////////////////////////////////////////////////////////
        ShaderPass* downSamplePass = m_shaderPasses["downsample"];
        cmd.bind_shaderpass(*downSamplePass);

        for (uint32_t i = 1; i < MIPMAP_LEVELS; i++)
        {

            Mipmap mipmap = {i - 1, i};

            cmd.push_constants(*downSamplePass, SHADER_STAGE_COMPUTE, &mipmap, sizeof(mipmap));

            cmd.bind_descriptor_set(m_imageDescriptorSet, 0, *downSamplePass, {}, BINDING_TYPE_COMPUTE);

            // Dispatch the compute shader
            uint32_t mipWidth  = std::max(1u, m_inAttachments[1]->extent.width >> i);
            uint32_t mipHeight = std::max(1u, m_inAttachments[1]->extent.height >> i);
            cmd.dispatch_compute({(mipWidth + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE, (mipHeight + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE, 1});

            cmd.pipeline_barrier(m_bloomMipmaps[mipmap.dstLevel],
                                 LAYOUT_GENERAL,
                                 LAYOUT_GENERAL,
                                 ACCESS_SHADER_WRITE,
                                 ACCESS_SHADER_READ,
                                 STAGE_COMPUTE_SHADER,
                                 STAGE_COMPUTE_SHADER);
        }

        cmd.pipeline_barrier(*m_inAttachments[1],
                             LAYOUT_UNDEFINED,
                             LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                             ACCESS_SHADER_WRITE,
                             ACCESS_SHADER_READ,
                             STAGE_FRAGMENT_SHADER,
                             STAGE_COMPUTE_SHADER);

        ////////////////////////////////////////////////////////////
        // UPSAMPLE
        ////////////////////////////////////////////////////////////
        ShaderPass* upSamplePass = m_shaderPasses["upsample"];
        cmd.bind_shaderpass(*upSamplePass);

        for (int32_t i = MIPMAP_LEVELS - 1; i > 0; i--)
        {

            Mipmap mipmap = {(uint32_t)i, (uint32_t)i - 1};

            cmd.push_constants(*upSamplePass, SHADER_STAGE_COMPUTE, &mipmap, sizeof(mipmap));

            cmd.bind_descriptor_set(m_imageDescriptorSet, 0, *upSamplePass, {}, BINDING_TYPE_COMPUTE);

            // Dispatch the compute shader
            uint32_t mipWidth  = std::max(1u, m_inAttachments[1]->extent.width >> (i - 1));
            uint32_t mipHeight = std::max(1u, m_inAttachments[1]->extent.height >> (i - 1));
            cmd.dispatch_compute({(mipWidth + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE, (mipHeight + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE, 1});

            cmd.pipeline_barrier(m_bloomMipmaps[mipmap.dstLevel],
                                 LAYOUT_GENERAL,
                                 LAYOUT_GENERAL,
                                 ACCESS_SHADER_WRITE,
                                 ACCESS_SHADER_READ,
                                 STAGE_COMPUTE_SHADER,
                                 STAGE_COMPUTE_SHADER);
        }

        // Prepare image to be read from
        cmd.pipeline_barrier(m_bloomImage, LAYOUT_GENERAL, LAYOUT_SHADER_READ_ONLY_OPTIMAL, ACCESS_SHADER_WRITE, ACCESS_SHADER_READ, STAGE_COMPUTE_SHADER);
    }

    ////////////////////////////////////////////////////////////
    // ADD BLOOM
    ////////////////////////////////////////////////////////////

    ShaderPass* shaderPass = m_shaderPasses["bloom"];

    cmd = view.commandBuffer;

    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);

    cmd.bind_shaderpass(*shaderPass);
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &m_bloomStrength, sizeof(float));
    cmd.bind_descriptor_set(m_imageDescriptorSet, 0, *shaderPass);

    cmd.draw_geometry(*get_VAO(shared.vignette));

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);
}

void BloomPass::link_input_attachments() {

    // m_originalImage = images[0];
    // *m_inAttachments   = images[1];

    ImageConfig config  = {};
    config.usageFlags   = IMAGE_USAGE_SAMPLED | IMAGE_USAGE_STORAGE | IMAGE_USAGE_TRANSFER_DST;
    config.mipLevels    = MIPMAP_LEVELS;
    config.baseMipLevel = 0;
    config.format       = m_colorFormat;
    m_bloomImage        = m_device->create_image(m_inAttachments[1]->extent, config);
    m_bloomImage.create_view(config);
    SamplerConfig samplerConfig      = {};
    samplerConfig.minLod             = 0;
    samplerConfig.maxLod             = MIPMAP_LEVELS;
    samplerConfig.samplerAddressMode = ADDRESS_MODE_CLAMP_TO_EDGE;
    m_bloomImage.create_sampler(samplerConfig);

    m_bloomMipmaps.resize(MIPMAP_LEVELS);
    // Create auxiliar images for mipmaps
    for (size_t i = 0; i < MIPMAP_LEVELS; i++)
    {
        m_bloomMipmaps[i]                     = m_bloomImage.clone();
        m_bloomMipmaps[i].config.baseMipLevel = i;
        m_bloomMipmaps[i].config.mipLevels    = 1;
        m_bloomMipmaps[i].create_view(config);
    }

    m_imageDescriptorSet.update(m_inAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    m_imageDescriptorSet.update(m_inAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    m_imageDescriptorSet.update(m_bloomMipmaps, LAYOUT_GENERAL, 2, UNIFORM_STORAGE_IMAGE);
    m_imageDescriptorSet.update(m_bloomMipmaps, LAYOUT_GENERAL, 3);
    m_imageDescriptorSet.update(&m_bloomImage, LAYOUT_SHADER_READ_ONLY_OPTIMAL, 4);
}

void BloomPass::resize_attachments() {
    BaseGraphicPass::resize_attachments();
    m_bloomImage.cleanup();
    for (Image& img : m_bloomMipmaps)
    {
        img.handle  = VK_NULL_HANDLE;
        img.sampler = VK_NULL_HANDLE;
        img.cleanup();
    }
}

void BloomPass::cleanup() {
    m_bloomImage.cleanup();
    for (Image& img : m_bloomMipmaps)
    {
        img.handle  = VK_NULL_HANDLE;
        img.sampler = VK_NULL_HANDLE;
        img.cleanup();
    }
    BaseGraphicPass::cleanup();
}

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END