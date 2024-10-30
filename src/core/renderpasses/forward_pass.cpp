#include <engine/core/renderpasses/forward_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core
{

void ForwardPass::init()
{

    VkSampleCountFlagBits samples = static_cast<VkSampleCountFlagBits>(m_aa);
    bool multisampled = samples > VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkAttachmentDescription> attachmentsInfo;

    // Color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = static_cast<VkFormat>(m_colorFormat);
    colorAttachment.samples = samples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout =
        m_isDefault ? (multisampled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachmentsInfo.push_back(colorAttachment);

    ImageConfig colorAttachmentImageConfig{};
    colorAttachmentImageConfig.format = static_cast<VkFormat>(m_colorFormat);
    colorAttachmentImageConfig.usageFlags =
        !m_isDefault ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
                     : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    colorAttachmentImageConfig.samples = samples;
    SamplerConfig colorAttachmentSamplerConfig{};
    colorAttachmentSamplerConfig.filters = VK_FILTER_LINEAR;
    colorAttachmentSamplerConfig.samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    Attachment _colorAttachment(colorAttachmentImageConfig, {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D},
                                colorAttachmentSamplerConfig);
    _colorAttachment.isPresentImage = m_isDefault ? (multisampled ? false : true) : false;
    m_attachments.push_back(_colorAttachment);

    VkAttachmentReference colorRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Resolve attachment
    if (multisampled)
    {
        VkAttachmentDescription resolveAttachment{};
        resolveAttachment.format = static_cast<VkFormat>(m_colorFormat);
        resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachmentsInfo.push_back(resolveAttachment);

        ImageConfig resolveAttachmentImageConfig{};
        resolveAttachmentImageConfig.format = static_cast<VkFormat>(m_colorFormat);
        resolveAttachmentImageConfig.usageFlags =
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        Attachment _resolveAttachment(resolveAttachmentImageConfig, {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D},
                                      {});
        _resolveAttachment.isPresentImage = true;
        m_attachments.push_back(_resolveAttachment);
    }
    VkAttachmentReference resolveRef = init::attachment_reference(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = static_cast<VkFormat>(m_depthFormat);
    depthAttachment.samples = samples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachmentsInfo.push_back(depthAttachment);

    ImageConfig depthAttachmentImageConfig{};
    depthAttachmentImageConfig.format = static_cast<VkFormat>(m_depthFormat);
    depthAttachmentImageConfig.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthAttachmentImageConfig.samples = samples;
    m_attachments.push_back(
        Attachment(depthAttachmentImageConfig, {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D}, {}));

    VkAttachmentReference depthRef =
        init::attachment_reference(attachmentsInfo.size() - 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;
    subpass.pResolveAttachments = multisampled ? &resolveRef : nullptr;

    // Depdencies
    std::array<VkSubpassDependency, 2> dependencies = {};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Creation
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
    renderPassInfo.pAttachments = attachmentsInfo.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(m_context->device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
}
void ForwardPass::create_descriptors()
{
    m_descriptorManager.init(m_context->device);
    m_descriptorManager.create_pool(VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS);
    m_descriptors.resize(m_context->frames.size());

    // GLOBAL SET
    VkDescriptorSetLayoutBinding camBufferBinding = init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding sceneBufferBinding = init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding shadowBinding = init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2); // ShadowMaps
    VkDescriptorSetLayoutBinding envBinding = init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3); // EnvMap
    VkDescriptorSetLayoutBinding iblBinding = init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4); // IrradianceMap
    VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding,
                                               iblBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 5);

    // PER-OBJECT SET
    VkDescriptorSetLayoutBinding objectBufferBinding = init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding materialBufferBinding = init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding objectBindings[] = {objectBufferBinding, materialBufferBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, objectBindings, 2);

    // MATERIAL TEXTURE SET
    VkDescriptorSetLayoutBinding textureBinding1 =
        init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding textureBinding2 =
        init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding textureBinding3 =
        init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    VkDescriptorSetLayoutBinding textureBinding4 =
        init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
    VkDescriptorSetLayoutBinding textureBinding5 =
        init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
    VkDescriptorSetLayoutBinding textureBindings[] = {textureBinding1, textureBinding2, textureBinding3,
                                                      textureBinding4, textureBinding5};
    m_descriptorManager.set_layout(DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, textureBindings, 5);

    for (size_t i = 0; i < m_context->frames.size(); i++)
    {
        // Global
        m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT,
                                                    &m_descriptors[i].globalDescritor);
        m_descriptorManager.set_descriptor_write(&m_context->frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                                 sizeof(CameraUniforms), 0, &m_descriptors[i].globalDescritor,
                                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);
        m_descriptorManager.set_descriptor_write(
            &m_context->frames[i].uniformBuffers[GLOBAL_LAYOUT], sizeof(SceneUniforms),
            utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_context->gpu), &m_descriptors[i].globalDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);

        m_descriptorManager.set_descriptor_write(
            get_image(Texture::DEBUG_TEXTURE)->sampler, get_image(Texture::DEBUG_TEXTURE)->view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3);

        // Per-object
        m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::OBJECT_LAYOUT,
                                                    &m_descriptors[i].objectDescritor);
        m_descriptorManager.set_descriptor_write(&m_context->frames[i].uniformBuffers[OBJECT_LAYOUT],
                                                 sizeof(ObjectUniforms), 0, &m_descriptors[i].objectDescritor,
                                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);
        m_descriptorManager.set_descriptor_write(
            &m_context->frames[i].uniformBuffers[OBJECT_LAYOUT], sizeof(MaterialUniforms),
            utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_context->gpu), &m_descriptors[i].objectDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);
    }
}
void ForwardPass::create_graphic_pipelines()
{

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                 VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                                                 VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, VK_DYNAMIC_STATE_CULL_MODE};
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{init::color_blend_attachment_state(true)};

    VkSampleCountFlagBits samples = static_cast<VkSampleCountFlagBits>(m_aa);

    // Setup shaderpasses
    m_shaderPasses["unlit"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/forward/unlit.glsl");
    m_shaderPasses["unlit"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["unlit"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                    {VertexAttributeType::NORMAL, false},
                                                    {VertexAttributeType::UV, false},
                                                    {VertexAttributeType::TANGENT, false},
                                                    {VertexAttributeType::COLOR, false}};
    m_shaderPasses["unlit"]->settings.blendAttachments = blendAttachments;
    m_shaderPasses["unlit"]->settings.dynamicStates = dynamicStates;
    m_shaderPasses["unlit"]->settings.samples = samples;

    m_shaderPasses["phong"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/forward/phong.glsl");
    m_shaderPasses["phong"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["phong"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                    {VertexAttributeType::NORMAL, true},
                                                    {VertexAttributeType::UV, true},
                                                    {VertexAttributeType::TANGENT, false},
                                                    {VertexAttributeType::COLOR, false}};
    m_shaderPasses["phong"]->settings.blendAttachments = blendAttachments;
    m_shaderPasses["phong"]->settings.dynamicStates = dynamicStates;
    m_shaderPasses["phong"]->settings.samples = samples;

    m_shaderPasses["physical"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/forward/physically_based.glsl");
    m_shaderPasses["physical"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                   {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                   {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["physical"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                       {VertexAttributeType::NORMAL, true},
                                                       {VertexAttributeType::UV, true},
                                                       {VertexAttributeType::TANGENT, true},
                                                       {VertexAttributeType::COLOR, false}};
    m_shaderPasses["physical"]->settings.blendAttachments = blendAttachments;
    m_shaderPasses["physical"]->settings.dynamicStates = dynamicStates;
    m_shaderPasses["physical"]->settings.samples = samples;

    m_shaderPasses["hair"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/forward/hair_strand.glsl");
    m_shaderPasses["hair"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                               {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                               {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["hair"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                   {VertexAttributeType::NORMAL, false},
                                                   {VertexAttributeType::UV, false},
                                                   {VertexAttributeType::TANGENT, true},
                                                   {VertexAttributeType::COLOR, true}};
    m_shaderPasses["hair"]->settings.dynamicStates = dynamicStates;
    m_shaderPasses["hair"]->settings.samples = samples;
    m_shaderPasses["hair"]->settings.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    m_shaderPasses["skybox"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/skybox.glsl");
    m_shaderPasses["skybox"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                 {DescriptorLayoutType::OBJECT_LAYOUT, false},
                                                                 {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["skybox"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                     {VertexAttributeType::NORMAL, false},
                                                     {VertexAttributeType::UV, false},
                                                     {VertexAttributeType::TANGENT, false},
                                                     {VertexAttributeType::COLOR, false}};
    m_shaderPasses["skybox"]->settings.dynamicStates = dynamicStates;
    m_shaderPasses["skybox"]->settings.samples = samples;
    m_shaderPasses["skybox"]->settings.blendAttachments = blendAttachments;
    // VkPushConstantRange pushConstantRange{};
    // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    // pushConstantRange.offset = 0;
    // pushConstantRange.size = sizeof(Graphics::SkyboxUniforms);
    // m_shaderPasses["skybox"]->settings.pushConstants = {pushConstantRange};
    // m_shaderPasses["skybox"]->settings.dynamicStates = {VK_DYNAMIC_STATE_DEPTH_COMPARE_OP};
    m_shaderPasses["skybox"]->settings.depthOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    for (auto pair : m_shaderPasses)
    {
        ShaderPass *pass = pair.second;

        ShaderPass::build_shader_stages(m_context->device, *pass);

        ShaderPass::build(m_context->device, m_handle, m_descriptorManager, m_extent, *pass);
    }
}

void ForwardPass::render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    PROFILING_EVENT()

    VkCommandBuffer cmd = m_context->frames[frameIndex].commandBuffer;

    begin(cmd, presentImageIndex);

    // Viewport setup
    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {

        unsigned int mesh_idx = 0;
        for (Mesh *m : scene->get_meshes())
        {
            if (m)
            {
                if (m->is_active() &&              // Check if is active
                    m->get_num_geometries() > 0 && // Check if has geometry
                    (scene->get_active_camera()->get_frustrum_culling() && m->get_bounding_volume()
                         ? m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum())
                         : true)) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset = m_context->frames[frameIndex].uniformBuffers[1].strideSize * mesh_idx;
                    uint32_t globalOffset = 0;

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        Geometry *g = m->get_geometry(i);
                        IMaterial *mat = m->get_material(g->get_material_ID());

                        // Setup per object render state

                        vkCmdSetDepthTestEnable(cmd, mat->get_parameters().depthTest);
                        vkCmdSetDepthWriteEnable(cmd, mat->get_parameters().depthWrite);
                        vkCmdSetCullMode(cmd, mat->get_parameters().faceCulling
                                                  ? (VkCullModeFlags)mat->get_parameters().culling
                                                  : VK_CULL_MODE_NONE);

                        ShaderPass *shaderPass = m_shaderPasses[mat->get_shaderpass_ID()];

                        // Bind pipeline
                        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);

                        // GLOBAL LAYOUT BINDING
                        uint32_t globalOffsets[] = {globalOffset, globalOffset};
                        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1,
                                                &m_descriptors[frameIndex].globalDescritor.handle, 2, globalOffsets);

                        // PER OBJECT LAYOUT BINDING
                        uint32_t objectOffsets[] = {objectOffset, objectOffset};
                        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 1, 1,
                                                &m_descriptors[frameIndex].objectDescritor.handle, 2, objectOffsets);

                        // TEXTURE LAYOUT BINDING
                        if (shaderPass->settings.descriptorSetLayoutIDs[DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT])
                            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 2,
                                                    1, &mat->get_texture_descriptor().handle, 0, nullptr);

                        draw(cmd, g);
                    }
                }
            }
            mesh_idx++;
        }
        // Skybox
        if (scene->get_skybox())
        {
            if (scene->get_skybox()->is_active())
            {
                vkCmdSetDepthTestEnable(cmd, VK_TRUE);
                vkCmdSetDepthWriteEnable(cmd, VK_TRUE);
                vkCmdSetCullMode(cmd,VK_CULL_MODE_NONE);

                ShaderPass *shaderPass = m_shaderPasses["skybox"];

                // Bind pipeline
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);

                // GLOBAL LAYOUT BINDING
                uint32_t globalOffsets[] = {0, 0};
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1,
                                        &m_descriptors[frameIndex].globalDescritor.handle, 2, globalOffsets);


                draw(cmd, scene->get_skybox()->get_box());
            }
        }
    }

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled && ImGui::GetDrawData())
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    end(cmd);
}

void ForwardPass::upload_data(uint32_t frameIndex, Scene *const scene)
{
    for (Mesh *m : scene->get_meshes())
    {
        if (m)
        {
            for (size_t i = 0; i < m->get_num_geometries(); i++)
            {
                Geometry *g = m->get_geometry(i);
                IMaterial *mat = m->get_material(g->get_material_ID());
                setup_material_descriptor(mat);
            }
        }
    }
}
void ForwardPass::connect_to_previous_images(std::vector<Image> images)
{
    for (size_t i = 0; i < m_context->frames.size(); i++)
    {
        m_descriptorManager.set_descriptor_write(images[0].sampler, images[0].view,
                                                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                                 &m_descriptors[i].globalDescritor, 2);
    }
}

void ForwardPass::set_envmap_descriptor(Graphics::Image env, Graphics::Image irr)
{
    for (size_t i = 0; i < m_context->frames.size(); i++)
    {
        m_descriptorManager.set_descriptor_write(env.sampler, env.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                 &m_descriptors[i].globalDescritor, 3);
        m_descriptorManager.set_descriptor_write(irr.sampler, irr.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                 &m_descriptors[i].globalDescritor, 4);
    }
}
void ForwardPass::setup_material_descriptor(IMaterial *mat)
{
    if (!mat->get_texture_descriptor().allocated)
        m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT,
                                                    &mat->get_texture_descriptor());

    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        ITexture *texture = pair.second;
        if (texture && texture->loaded_on_GPU())
        {

            // Set texture write
            if (!mat->get_texture_binding_state()[pair.first] || texture->is_dirty())
            {
                m_descriptorManager.set_descriptor_write(get_image(texture)->sampler, get_image(texture)->view,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                         &mat->get_texture_descriptor(), pair.first);
                mat->set_texture_binding_state(pair.first, true);
                texture->set_dirty(false);
            }
        }
        else
        {
            // SET DUMMY TEXTURE
            if (!mat->get_texture_binding_state()[pair.first])
                m_descriptorManager.set_descriptor_write(
                    get_image(Texture::DEBUG_TEXTURE)->sampler, get_image(Texture::DEBUG_TEXTURE)->view,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->get_texture_descriptor(), pair.first);
            mat->set_texture_binding_state(pair.first, true);
        }
    }
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END