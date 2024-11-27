#include <engine/core/renderpasses/forward_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void ForwardPass::setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                                    std::vector<Graphics::SubPassDependency>& dependencies) {

    uint16_t samples      = static_cast<uint16_t>(m_aa);
    bool     multisampled = samples > 1;

    Graphics::Attachment colorAttachment =
        Graphics::Attachment(m_colorFormat,
                             samples,
                             m_isDefault ? (multisampled ? LAYOUT_COLOR_ATTACHMENT_OPTIMAL : LAYOUT_PRESENT)
                                         : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                             LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             !m_isDefault ? IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED
                                          : IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT,
                             COLOR_ATTACHMENT,
                             ASPECT_COLOR,
                             TEXTURE_2D,
                             FILTER_LINEAR,
                             ADDRESS_MODE_CLAMP_TO_BORDER);
    colorAttachment.isPresentImage = m_isDefault ? (multisampled ? false : true) : false;
    attachments.push_back(colorAttachment);

    Graphics::Attachment resolveAttachment;
    if (multisampled)
    {
        resolveAttachment                = Graphics::Attachment(m_colorFormat,
                                                 1,
                                                 LAYOUT_PRESENT,
                                                 LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                 IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT,
                                                 RESOLVE_ATTACHMENT,
                                                 ASPECT_COLOR,
                                                 TEXTURE_2D);
        resolveAttachment.isPresentImage = multisampled ? true : false;
        attachments.push_back(resolveAttachment);
    }

    Graphics::Attachment depthAttachment = Graphics::Attachment(m_depthFormat,
                                                                samples,
                                                                LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
                                                                DEPTH_ATTACHMENT,
                                                                ASPECT_DEPTH,
                                                                TEXTURE_2D);
    attachments.push_back(depthAttachment);

    // Depdencies
    dependencies.resize(2);
    // STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_FRAGMENT_SHADER, ACCESS_SHADER_READ);

    dependencies[0] = Graphics::SubPassDependency(
        STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[1] = Graphics::SubPassDependency(
        STAGE_EARLY_FRAGMENT_TESTS, STAGE_EARLY_FRAGMENT_TESTS, ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE);
}
void ForwardPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    m_descriptorPool = m_device->create_descriptor_pool(
        ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding camBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding sceneBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding shadowBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding envBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding iblBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding accelBinding(UNIFORM_ACCELERATION_STRUCTURE, SHADER_STAGE_FRAGMENT, 5);
    LayoutBinding noiseBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 6);
    m_descriptorPool.set_layout(
        GLOBAL_LAYOUT,
        {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding, accelBinding, noiseBinding});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding materialBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    // MATERIAL TEXTURE SET
    LayoutBinding textureBinding1(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding textureBinding2(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding textureBinding3(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding textureBinding4(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding textureBinding5(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding textureBinding6(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5);
    m_descriptorPool.set_layout(
        OBJECT_TEXTURE_LAYOUT,
        {textureBinding1, textureBinding2, textureBinding3, textureBinding4, textureBinding5, textureBinding6});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(SceneUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);

        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_TEXTURE),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);

        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::BLUE_NOISE_TEXTURE),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              6);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                              sizeof(ObjectUniforms),
                                              0,
                                              &m_descriptors[i].objectDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                              sizeof(MaterialUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(MaterialUniforms)),
                                              &m_descriptors[i].objectDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);
        // Set up enviroment fallback texture
        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);
        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
    m_shaderPasses["unlit"]->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["unlit"]->settings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                          {NORMAL_ATTRIBUTE, false},
                                                          {UV_ATTRIBUTE, false},
                                                          {TANGENT_ATTRIBUTE, false},
                                                          {COLOR_ATTRIBUTE, false}};
    m_shaderPasses["unlit"]->settings.blendAttachments = blendAttachments;
    m_shaderPasses["unlit"]->settings.dynamicStates    = dynamicStates;
    m_shaderPasses["unlit"]->settings.samples          = samples;

    m_shaderPasses["phong"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/phong.glsl");
    m_shaderPasses["phong"]->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["phong"]->settings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                          {NORMAL_ATTRIBUTE, true},
                                                          {UV_ATTRIBUTE, true},
                                                          {TANGENT_ATTRIBUTE, false},
                                                          {COLOR_ATTRIBUTE, false}};
    m_shaderPasses["phong"]->settings.blendAttachments = blendAttachments;
    m_shaderPasses["phong"]->settings.dynamicStates    = dynamicStates;
    m_shaderPasses["phong"]->settings.samples          = samples;

    m_shaderPasses["physical"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/physically_based.glsl");
    m_shaderPasses["physical"]->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["physical"]->settings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                             {NORMAL_ATTRIBUTE, true},
                                                             {UV_ATTRIBUTE, true},
                                                             {TANGENT_ATTRIBUTE, true},
                                                             {COLOR_ATTRIBUTE, false}};
    m_shaderPasses["physical"]->settings.blendAttachments = blendAttachments;
    m_shaderPasses["physical"]->settings.dynamicStates    = dynamicStates;
    m_shaderPasses["physical"]->settings.samples          = samples;

    m_shaderPasses["hairstr"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/hair_strand.glsl");
    m_shaderPasses["hairstr"]->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["hairstr"]->settings.attributes    = {{POSITION_ATTRIBUTE, true},
                                                         {NORMAL_ATTRIBUTE, false},
                                                         {UV_ATTRIBUTE, false},
                                                         {TANGENT_ATTRIBUTE, true},
                                                         {COLOR_ATTRIBUTE, true}};
    m_shaderPasses["hairstr"]->settings.dynamicStates = dynamicStates;
    m_shaderPasses["hairstr"]->settings.samples       = samples;
    m_shaderPasses["hairstr"]->settings.sampleShading = false;
    m_shaderPasses["hairstr"]->settings.topology      = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    m_shaderPasses["hairstr2"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/hair_strand2.glsl");
    m_shaderPasses["hairstr2"]->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    m_shaderPasses["hairstr2"]->settings.attributes    = {{POSITION_ATTRIBUTE, true},
                                                          {NORMAL_ATTRIBUTE, false},
                                                          {UV_ATTRIBUTE, false},
                                                          {TANGENT_ATTRIBUTE, true},
                                                          {COLOR_ATTRIBUTE, true}};
    m_shaderPasses["hairstr2"]->settings.dynamicStates = dynamicStates;
    m_shaderPasses["hairstr2"]->settings.samples       = samples;
    m_shaderPasses["hairstr2"]->settings.sampleShading = false;
    m_shaderPasses["hairstr2"]->settings.topology      = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    m_shaderPasses["skybox"] =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/forward/skybox.glsl");
    m_shaderPasses["skybox"]->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, false}, {OBJECT_TEXTURE_LAYOUT, false}};
    m_shaderPasses["skybox"]->settings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                           {NORMAL_ATTRIBUTE, false},
                                                           {UV_ATTRIBUTE, false},
                                                           {TANGENT_ATTRIBUTE, false},
                                                           {COLOR_ATTRIBUTE, false}};
    m_shaderPasses["skybox"]->settings.dynamicStates    = dynamicStates;
    m_shaderPasses["skybox"]->settings.samples          = samples;
    m_shaderPasses["skybox"]->settings.blendAttachments = blendAttachments;
    m_shaderPasses["skybox"]->settings.depthOp          = VK_COMPARE_OP_LESS_OR_EQUAL;

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
                        if (shaderPass->settings.descriptorSetLayoutIDs[OBJECT_TEXTURE_LAYOUT])
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
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              2);
    }
}

void ForwardPass::set_envmap_descriptor(Graphics::Image env, Graphics::Image irr) {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        m_descriptorPool.set_descriptor_write(
            &env, LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3);
        m_descriptorPool.set_descriptor_write(
            &irr, LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 4);
    }
}
void ForwardPass::setup_material_descriptor(IMaterial* mat) {
    if (!mat->get_texture_descriptor().allocated)
        m_descriptorPool.allocate_descriptor_set(OBJECT_TEXTURE_LAYOUT, &mat->get_texture_descriptor());

    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        ITexture* texture = pair.second;
        if (texture && texture->loaded_on_GPU())
        {

            // Set texture write
            if (!mat->get_texture_binding_state()[pair.first] || texture->is_dirty())
            {
                m_descriptorPool.set_descriptor_write(
                    get_image(texture), LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->get_texture_descriptor(), pair.first);
                mat->set_texture_binding_state(pair.first, true);
                texture->set_dirty(false);
            }
        } else
        {
            // SET DUMMY TEXTURE
            if (!mat->get_texture_binding_state()[pair.first])
                m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_TEXTURE),
                                                      LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                      &mat->get_texture_descriptor(),
                                                      pair.first);
            mat->set_texture_binding_state(pair.first, true);
        }
    }
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END