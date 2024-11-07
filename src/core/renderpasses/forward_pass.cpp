#include <engine/core/renderpasses/forward_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void ForwardPass::setup_attachments() {
    VkSampleCountFlagBits samples      = static_cast<VkSampleCountFlagBits>(m_aa);
    bool                  multisampled = samples > VK_SAMPLE_COUNT_1_BIT;

    Graphics::Attachment colorAttachment = Graphics::Attachment(
        static_cast<VkFormat>(m_colorFormat),
        samples,
        m_isDefault ? (multisampled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        !m_isDefault ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
                     : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        AttachmentType::COLOR_ATTACHMENT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FILTER_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    colorAttachment.isPresentImage = m_isDefault ? (multisampled ? false : true) : false;
    m_attachments.push_back(colorAttachment);

    Graphics::Attachment resolveAttachment;
    if (multisampled)
    {
        resolveAttachment =
            Graphics::Attachment(static_cast<VkFormat>(m_colorFormat),
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 AttachmentType::RESOLVE_ATTACHMENT,
                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                 VK_IMAGE_VIEW_TYPE_2D);
        resolveAttachment.isPresentImage =  multisampled ? true : false;
        m_attachments.push_back(resolveAttachment);
    }

    Graphics::Attachment depthAttachment = Graphics::Attachment(static_cast<VkFormat>(m_depthFormat),
                                                                samples,
                                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                AttachmentType::DEPTH_ATTACHMENT,
                                                                VK_IMAGE_ASPECT_DEPTH_BIT,
                                                                VK_IMAGE_VIEW_TYPE_2D);
    m_attachments.push_back(depthAttachment);

    // Depdencies
    m_dependencies.resize(2);

    m_dependencies[0] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    m_dependencies[1] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

  
}
void ForwardPass::setup_uniforms() {

    m_device->create_descriptor_pool(
        m_descriptorPool, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS);
    m_descriptors.resize(RenderPass::frames.size());

    // GLOBAL SET
    VkDescriptorSetLayoutBinding camBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    VkDescriptorSetLayoutBinding sceneBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    VkDescriptorSetLayoutBinding shadowBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2); // ShadowMaps
    VkDescriptorSetLayoutBinding envBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3); // EnvMap
    VkDescriptorSetLayoutBinding iblBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4); // IrradianceMap
    VkDescriptorSetLayoutBinding bindings[] = {
        camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding};
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 5);

    // PER-OBJECT SET
    VkDescriptorSetLayoutBinding objectBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    VkDescriptorSetLayoutBinding materialBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    VkDescriptorSetLayoutBinding objectBindings[] = {objectBufferBinding, materialBufferBinding};
    m_descriptorPool.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, objectBindings, 2);

    // MATERIAL TEXTURE SET
    VkDescriptorSetLayoutBinding textureBinding1 =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding textureBinding2 =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding textureBinding3 =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    VkDescriptorSetLayoutBinding textureBinding4 =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
    VkDescriptorSetLayoutBinding textureBinding5 =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
    VkDescriptorSetLayoutBinding textureBindings[] = {
        textureBinding1, textureBinding2, textureBinding3, textureBinding4, textureBinding5};
    m_descriptorPool.set_layout(DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, textureBindings, 5);

    for (size_t i = 0; i < RenderPass::frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&RenderPass::frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(
            &RenderPass::frames[i].uniformBuffers[GLOBAL_LAYOUT],
            sizeof(SceneUniforms),
            Utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_device->get_GPU()),
            &m_descriptors[i].globalDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1);

        m_descriptorPool.set_descriptor_write(get_image(Texture::FALLBACK_TEX)->sampler,
                                              get_image(Texture::FALLBACK_TEX)->view,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.set_descriptor_write(&RenderPass::frames[i].uniformBuffers[OBJECT_LAYOUT],
                                              sizeof(ObjectUniforms),
                                              0,
                                              &m_descriptors[i].objectDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(
            &RenderPass::frames[i].uniformBuffers[OBJECT_LAYOUT],
            sizeof(MaterialUniforms),
            Utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_device->get_GPU()),
            &m_descriptors[i].objectDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1);
    }
}
void ForwardPass::setup_shader_passes() {

    std::vector<VkDynamicState>                      dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                                      VK_DYNAMIC_STATE_SCISSOR,
                                                                      VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                                                                      VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                                                                      VK_DYNAMIC_STATE_CULL_MODE};
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{Init::color_blend_attachment_state(true)};

    VkSampleCountFlagBits samples = static_cast<VkSampleCountFlagBits>(m_aa);

    // Setup shaderpasses
    m_shaderPasses["unlit"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/unlit.glsl");
    m_shaderPasses["unlit"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["unlit"]->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                                {VertexAttributeType::NORMAL, false},
                                                                {VertexAttributeType::UV, false},
                                                                {VertexAttributeType::TANGENT, false},
                                                                {VertexAttributeType::COLOR, false}};
    m_shaderPasses["unlit"]->settings.blendAttachments       = blendAttachments;
    m_shaderPasses["unlit"]->settings.dynamicStates          = dynamicStates;
    m_shaderPasses["unlit"]->settings.samples                = samples;

    m_shaderPasses["phong"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/phong.glsl");
    m_shaderPasses["phong"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["phong"]->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                                {VertexAttributeType::NORMAL, true},
                                                                {VertexAttributeType::UV, true},
                                                                {VertexAttributeType::TANGENT, false},
                                                                {VertexAttributeType::COLOR, false}};
    m_shaderPasses["phong"]->settings.blendAttachments       = blendAttachments;
    m_shaderPasses["phong"]->settings.dynamicStates          = dynamicStates;
    m_shaderPasses["phong"]->settings.samples                = samples;

    m_shaderPasses["physical"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/physically_based.glsl");
    m_shaderPasses["physical"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                   {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                   {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["physical"]->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                                   {VertexAttributeType::NORMAL, true},
                                                                   {VertexAttributeType::UV, true},
                                                                   {VertexAttributeType::TANGENT, true},
                                                                   {VertexAttributeType::COLOR, false}};
    m_shaderPasses["physical"]->settings.blendAttachments       = blendAttachments;
    m_shaderPasses["physical"]->settings.dynamicStates          = dynamicStates;
    m_shaderPasses["physical"]->settings.samples                = samples;

    m_shaderPasses["hairstr"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/hair_strand.glsl");
    m_shaderPasses["hairstr"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                  {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                  {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["hairstr"]->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                                  {VertexAttributeType::NORMAL, false},
                                                                  {VertexAttributeType::UV, false},
                                                                  {VertexAttributeType::TANGENT, true},
                                                                  {VertexAttributeType::COLOR, true}};
    m_shaderPasses["hairstr"]->settings.dynamicStates          = dynamicStates;
    m_shaderPasses["hairstr"]->settings.samples                = samples;
    m_shaderPasses["hairstr"]->settings.sampleShading          = false;
    m_shaderPasses["hairstr"]->settings.topology               = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    m_shaderPasses["hairstr2"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/hair_strand2.glsl");
    m_shaderPasses["hairstr2"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                   {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                   {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["hairstr2"]->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                                   {VertexAttributeType::NORMAL, false},
                                                                   {VertexAttributeType::UV, false},
                                                                   {VertexAttributeType::TANGENT, true},
                                                                   {VertexAttributeType::COLOR, true}};
    m_shaderPasses["hairstr2"]->settings.dynamicStates          = dynamicStates;
    m_shaderPasses["hairstr2"]->settings.samples                = samples;
    m_shaderPasses["hairstr2"]->settings.sampleShading          = false;
    m_shaderPasses["hairstr2"]->settings.topology               = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    m_shaderPasses["skybox"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/skybox.glsl");
    m_shaderPasses["skybox"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                 {DescriptorLayoutType::OBJECT_LAYOUT, false},
                                                                 {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["skybox"]->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                                 {VertexAttributeType::NORMAL, false},
                                                                 {VertexAttributeType::UV, false},
                                                                 {VertexAttributeType::TANGENT, false},
                                                                 {VertexAttributeType::COLOR, false}};
    m_shaderPasses["skybox"]->settings.dynamicStates          = dynamicStates;
    m_shaderPasses["skybox"]->settings.samples                = samples;
    m_shaderPasses["skybox"]->settings.blendAttachments       = blendAttachments;
    m_shaderPasses["skybox"]->settings.depthOp                = VK_COMPARE_OP_LESS_OR_EQUAL;

    for (auto pair : m_shaderPasses)
    {
        ShaderPass* pass = pair.second;

        pass->build_shader_stages();
        pass->build(m_handle, m_descriptorPool, m_extent);
    }
}

void ForwardPass::render(uint32_t frameIndex, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    VkCommandBuffer cmd = RenderPass::frames[frameIndex].commandBuffer;

    begin(cmd, presentImageIndex);

    // Viewport setup
    VkViewport viewport = Init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {

        unsigned int mesh_idx = 0;
        for (Mesh* m : scene->get_meshes())
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
                    uint32_t objectOffset =
                        RenderPass::frames[frameIndex].uniformBuffers[1].get_stride_size() * mesh_idx;
                    uint32_t globalOffset = 0;

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        Geometry*  g   = m->get_geometry(i);
                        IMaterial* mat = m->get_material(g->get_material_ID());

                        // Setup per object render state

                        vkCmdSetDepthTestEnable(cmd, mat->get_parameters().depthTest);
                        vkCmdSetDepthWriteEnable(cmd, mat->get_parameters().depthWrite);
                        vkCmdSetCullMode(cmd,
                                         mat->get_parameters().faceCulling
                                             ? (VkCullModeFlags)mat->get_parameters().culling
                                             : VK_CULL_MODE_NONE);

                        ShaderPass* shaderPass = m_shaderPasses[mat->get_shaderpass_ID()];

                        // Bind pipeline
                        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->get_pipeline());

                        // GLOBAL LAYOUT BINDING
                        uint32_t globalOffsets[] = {globalOffset, globalOffset};
                        vkCmdBindDescriptorSets(cmd,
                                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                shaderPass->get_layout(),
                                                0,
                                                1,
                                                &m_descriptors[frameIndex].globalDescritor.handle,
                                                2,
                                                globalOffsets);

                        // PER OBJECT LAYOUT BINDING
                        uint32_t objectOffsets[] = {objectOffset, objectOffset};
                        vkCmdBindDescriptorSets(cmd,
                                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                shaderPass->get_layout(),
                                                1,
                                                1,
                                                &m_descriptors[frameIndex].objectDescritor.handle,
                                                2,
                                                objectOffsets);

                        // TEXTURE LAYOUT BINDING
                        if (shaderPass->settings.descriptorSetLayoutIDs[DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT])
                            vkCmdBindDescriptorSets(cmd,
                                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    shaderPass->get_layout(),
                                                    2,
                                                    1,
                                                    &mat->get_texture_descriptor().handle,
                                                    0,
                                                    nullptr);

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
                vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);

                ShaderPass* shaderPass = m_shaderPasses["skybox"];

                // Bind pipeline
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->get_pipeline());

                // GLOBAL LAYOUT BINDING
                uint32_t globalOffsets[] = {0, 0};
                vkCmdBindDescriptorSets(cmd,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        shaderPass->get_layout(),
                                        0,
                                        1,
                                        &m_descriptors[frameIndex].globalDescritor.handle,
                                        2,
                                        globalOffsets);

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

void ForwardPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    for (Mesh* m : scene->get_meshes())
    {
        if (m)
        {
            for (size_t i = 0; i < m->get_num_geometries(); i++)
            {
                Geometry*  g   = m->get_geometry(i);
                IMaterial* mat = m->get_material(g->get_material_ID());
                setup_material_descriptor(mat);
            }
        }
    }
}
void ForwardPass::connect_to_previous_images(std::vector<Image> images) {
    for (size_t i = 0; i < RenderPass::frames.size(); i++)
    {
        m_descriptorPool.set_descriptor_write(images[0].sampler,
                                              images[0].view,
                                              //   VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              2);
    }
}

void ForwardPass::set_envmap_descriptor(Graphics::Image env, Graphics::Image irr) {
    for (size_t i = 0; i < RenderPass::frames.size(); i++)
    {
        m_descriptorPool.set_descriptor_write(
            env.sampler, env.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3);
        m_descriptorPool.set_descriptor_write(
            irr.sampler, irr.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 4);
    }
}
void ForwardPass::setup_material_descriptor(IMaterial* mat) {
    if (!mat->get_texture_descriptor().allocated)
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, &mat->get_texture_descriptor());

    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        ITexture* texture = pair.second;
        if (texture && texture->loaded_on_GPU())
        {

            // Set texture write
            if (!mat->get_texture_binding_state()[pair.first] || texture->is_dirty())
            {
                m_descriptorPool.set_descriptor_write(get_image(texture)->sampler,
                                                      get_image(texture)->view,
                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                      &mat->get_texture_descriptor(),
                                                      pair.first);
                mat->set_texture_binding_state(pair.first, true);
                texture->set_dirty(false);
            }
        } else
        {
            // SET DUMMY TEXTURE
            if (!mat->get_texture_binding_state()[pair.first])
                m_descriptorPool.set_descriptor_write(get_image(Texture::FALLBACK_TEX)->sampler,
                                                      get_image(Texture::FALLBACK_TEX)->view,
                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                      &mat->get_texture_descriptor(),
                                                      pair.first);
            mat->set_texture_binding_state(pair.first, true);
        }
    }
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END