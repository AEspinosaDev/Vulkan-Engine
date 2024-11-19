#include <engine/core/renderpasses/forward_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void ForwardPass::setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                                    std::vector<Graphics::SubPassDependency>& dependencies) {

    uint16_t samples      = static_cast<uint16_t>(m_aa);
    bool     multisampled = samples > 1;

    Graphics::Attachment colorAttachment = Graphics::Attachment(
        m_colorFormat,
        samples,
        m_isDefault ? (multisampled ? ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL : ImageLayoutType::PRESENT)
                    : ImageLayoutType::SHADER_READ_ONLY_OPTIMAL,
        ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL,
        !m_isDefault ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
                     : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        AttachmentType::COLOR_ATTACHMENT,
        AspectType::COLOR,
        TextureType::TEXTURE_2D,
        FilterType::LINEAR,
        AddressMode::CLAMP_TO_BORDER);
    colorAttachment.isPresentImage = m_isDefault ? (multisampled ? false : true) : false;
    attachments.push_back(colorAttachment);

    Graphics::Attachment resolveAttachment;
    if (multisampled)
    {
        resolveAttachment =
            Graphics::Attachment(m_colorFormat,
                                 1,
                                 ImageLayoutType::PRESENT,
                                 ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 AttachmentType::RESOLVE_ATTACHMENT,
                                 AspectType::COLOR,
                                 TextureType::TEXTURE_2D);
        resolveAttachment.isPresentImage = multisampled ? true : false;
        attachments.push_back(resolveAttachment);
    }

    Graphics::Attachment depthAttachment = Graphics::Attachment(m_depthFormat,
                                                                samples,
                                                                ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                AttachmentType::DEPTH_ATTACHMENT,
                                                                AspectType::DEPTH,
                                                                TextureType::TEXTURE_2D);
    attachments.push_back(depthAttachment);

    // Depdencies
    dependencies.resize(2);

    dependencies[0] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    dependencies[1] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
}
void ForwardPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    m_descriptorPool = m_device->create_descriptor_pool(
        VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding camBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    LayoutBinding sceneBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    LayoutBinding shadowBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    LayoutBinding envBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
    LayoutBinding iblBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
    LayoutBinding accelBinding(UniformDataType::ACCELERATION_STRUCTURE, VK_SHADER_STAGE_FRAGMENT_BIT, 5);
    LayoutBinding noiseBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6);
    m_descriptorPool.set_layout(
        DescriptorLayoutType::GLOBAL_LAYOUT,
        {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding, accelBinding, noiseBinding});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    LayoutBinding materialBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    m_descriptorPool.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    // MATERIAL TEXTURE SET
    LayoutBinding textureBinding1(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    LayoutBinding textureBinding2(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    LayoutBinding textureBinding3(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    LayoutBinding textureBinding4(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
    LayoutBinding textureBinding5(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
    m_descriptorPool.set_layout(DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT,
                                {textureBinding1, textureBinding2, textureBinding3, textureBinding4, textureBinding5});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(SceneUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                              &m_descriptors[i].globalDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              1);

        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_TEXTURE),
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);

        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::BLUE_NOISE_TEXTURE),
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              6);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                              sizeof(ObjectUniforms),
                                              0,
                                              &m_descriptors[i].objectDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                              sizeof(MaterialUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(MaterialUniforms)),
                                              &m_descriptors[i].objectDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              1);
        // Set up enviroment fallback texture
        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);
        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              4);
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
        pass->build(m_handle, m_descriptorPool);
    }
}

void ForwardPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_handle, m_framebuffers[presentImageIndex]);
    cmd.set_viewport(m_handle.extent);

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
                    uint32_t objectOffset = currentFrame.uniformBuffers[1].strideSize * mesh_idx;

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        Geometry*  g   = m->get_geometry(i);
                        IMaterial* mat = m->get_material(g->get_material_ID());

                        cmd.set_depth_test_enable(mat->get_parameters().depthTest);
                        cmd.set_depth_write_enable(mat->get_parameters().depthWrite);
                        cmd.set_cull_mode(mat->get_parameters().faceCulling ? mat->get_parameters().culling
                                                                            : CullingMode::NO_CULLING);

                        ShaderPass* shaderPass = m_shaderPasses[mat->get_shaderpass_ID()];

                        // Bind pipeline
                        cmd.bind_shaderpass(*shaderPass);
                        // GLOBAL LAYOUT BINDING
                        cmd.bind_descriptor_set(
                            m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
                        // PER OBJECT LAYOUT BINDING
                        cmd.bind_descriptor_set(m_descriptors[currentFrame.index].objectDescritor,
                                                1,
                                                *shaderPass,
                                                {objectOffset, objectOffset});
                        // TEXTURE LAYOUT BINDING
                        if (shaderPass->settings.descriptorSetLayoutIDs[DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT])
                            cmd.bind_descriptor_set(mat->get_texture_descriptor(), 2, *shaderPass);

                        // DRAW
                        cmd.draw_geometry(*get_VAO(g));
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

                cmd.set_depth_test_enable(true);
                cmd.set_depth_write_enable(true);
                cmd.set_cull_mode(CullingMode::NO_CULLING);

                ShaderPass* shaderPass = m_shaderPasses["skybox"];

                // Bind pipeline
                cmd.bind_shaderpass(*shaderPass);

                // GLOBAL LAYOUT BINDING
                cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});

                cmd.draw_geometry(*get_VAO(scene->get_skybox()->get_box()));
            }
        }
    }

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        cmd.draw_gui_data();

    cmd.end_renderpass();
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
    if (!get_TLAS(scene)->binded)
    {
        for (size_t i = 0; i < m_descriptors.size(); i++)
        {
            m_descriptorPool.set_descriptor_write(get_TLAS(scene), &m_descriptors[i].globalDescritor, 5);
        }
        get_TLAS(scene)->binded = true;
    }
}
void ForwardPass::connect_to_previous_images(std::vector<Image> images) {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        m_descriptorPool.set_descriptor_write(&images[0],

                                              //   VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              2);
    }
}

void ForwardPass::set_envmap_descriptor(Graphics::Image env, Graphics::Image irr) {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        m_descriptorPool.set_descriptor_write(
            &env, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3);
        m_descriptorPool.set_descriptor_write(
            &irr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 4);
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
                m_descriptorPool.set_descriptor_write(get_image(texture),
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
                m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_TEXTURE),
                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                      &mat->get_texture_descriptor(),
                                                      pair.first);
            mat->set_texture_binding_state(pair.first, true);
        }
    }
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END