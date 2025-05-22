/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef POSTPROCESS_PASS_H
#define POSTPROCESS_PASS_H
#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {
/*
Generic Postprocess Pass. It recieves an image from a previous pass and performs a postptocess task defined by a shader
on it. Can be inherited.
*/
template <std::size_t numberIN, std::size_t numberOUT> class PostProcessPass : public BaseGraphicPass
{
  protected:
    ColorFormatType         m_colorFormat;
    Graphics::DescriptorSet m_imageDescriptorSet;
    std::string             m_shaderPath;

  public:
    PostProcessPass(const ptr<Graphics::Device>&            device,
                    const PassLinkage<numberIN, numberOUT>& config,
                    Extent2D                                extent,
                    ColorFormatType                         colorFormat,
                    std::string                             shaderPath,
                    std::string                             name      = "POST-PROCESS",
                    bool                                    isDefault = false)
        : BaseGraphicPass(device, extent, 1, 1, true, isDefault, name)
        , m_colorFormat(colorFormat)
        , m_shaderPath(shaderPath) {
        BasePass::store_attachments<numberIN, numberOUT>(config);
    }

    virtual void setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies);

    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames,const Render::Resources& shared);

    virtual void setup_shader_passes();

   virtual void execute(const Render::RenderView& view, const Render::Resources& shared) ;

    virtual void link_input_attachments();
};

#pragma region Implementation

template <std::size_t numberIN, std::size_t numberOUT>
void PostProcessPass<numberIN, numberOUT>::setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                                                                 std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::AttachmentConfig(
        m_colorFormat,
        1,
        this->m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        this->m_isDefault ? IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT : IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
        COLOR_ATTACHMENT,
        ASPECT_COLOR,
        TEXTURE_2D,
        FILTER_LINEAR,
        ADDRESS_MODE_CLAMP_TO_EDGE);

    attachments[0].isDefault = this->m_isDefault ? true : false;
    // attachments[0].imageConfig.mipLevels  = static_cast<uint32_t>(std::floor(std::log2(std::max(this->m_imageExtent.width, this->m_imageExtent.height)))) +
    // 1; attachments[0].imageConfig.useMipmaps = true;

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
template <std::size_t numberIN, std::size_t numberOUT> void PostProcessPass<numberIN, numberOUT>::setup_uniforms(std::vector<Graphics::Frame>& frames,const Render::Resources& shared) {
    // Init and configure local descriptors
    this->m_descriptorPool = this->m_device->create_descriptor_pool(1, numberIN, 1, 1, 1);

    // Do this for every input image atachment
    std::vector<VKFW::Graphics::LayoutBinding> bindings;
    for (size_t i = 0; i < numberIN; i++)
    {
        bindings.push_back(Graphics::LayoutBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, i));
    }
    this->m_descriptorPool.set_layout(GLOBAL_LAYOUT, bindings);

    this->m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &this->m_imageDescriptorSet);
}
template <std::size_t numberIN, std::size_t numberOUT> void PostProcessPass<numberIN, numberOUT>::setup_shader_passes() {

    Graphics::GraphicShaderPass* ppPass =
        new Graphics::GraphicShaderPass(this->m_device->get_handle(), this->m_renderpass, this->m_imageExtent, this->m_shaderPath);
    ppPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    ppPass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, false}, {COLOR_ATTRIBUTE, false}};

    ppPass->compile_shader_stages();
    ppPass->build(this->m_descriptorPool);

    this->m_shaderPasses["pp"] = ppPass;
}
template <std::size_t numberIN, std::size_t numberOUT>
void PostProcessPass<numberIN, numberOUT>::execute(const Render::RenderView& view, const Render::Resources& shared) {
    PROFILING_EVENT()

    Graphics::CommandBuffer cmd = view.commandBuffer;
    cmd.begin_renderpass(this->m_renderpass, this->m_framebuffers[this->m_isDefault ? view.presentImageIndex : 0]);
    cmd.set_viewport(this->m_imageExtent);

    Graphics::ShaderPass* shaderPass = this->m_shaderPasses["pp"];

    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_imageDescriptorSet, 0, *shaderPass);

    cmd.draw_geometry(*get_VAO(shared.vignette));

    // Draw gui contents
    // if (this->m_isDefault && Graphics::Frame::guiEnabled)
    //     cmd.draw_gui_data();

    cmd.end_renderpass(this->m_renderpass, this->m_framebuffers[this->m_isDefault ? view.presentImageIndex : 0]);
}
template <std::size_t numberIN, std::size_t numberOUT> void PostProcessPass<numberIN, numberOUT>::link_input_attachments() {
    for (size_t i = 0; i < numberIN; i++)
    {
        this->m_imageDescriptorSet.update(this->m_inAttachments[i], LAYOUT_SHADER_READ_ONLY_OPTIMAL,  i);
    }
}

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif