#include <engine/core/passes/forward_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void ForwardPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                                        std::vector<Graphics::SubPassDependency>& dependencies) {

    uint16_t samples      = static_cast<uint16_t>(m_aa);
    bool     multisampled = samples > 1;

    attachments.resize(multisampled ? 4 : 2);

    attachments[0] =
        Graphics::AttachmentConfig(m_colorFormat,
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
                                   ADDRESS_MODE_CLAMP_TO_EDGE);

    attachments[0].isDefault = m_isDefault ? (multisampled ? false : true) : false;

    // Bright color buffer. m_colorFormat should be in floating point.
    attachments[1] = Graphics::AttachmentConfig(m_colorFormat,
                                                samples,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                                COLOR_ATTACHMENT,
                                                ASPECT_COLOR,
                                                TEXTURE_2D,
                                                FILTER_LINEAR,
                                                ADDRESS_MODE_CLAMP_TO_EDGE);

    Graphics::AttachmentConfig resolveAttachment;
    if (multisampled)
    {
        m_interAttachments.resize(2);
        attachments[2] =
            Graphics::AttachmentConfig(m_colorFormat,
                                       1,
                                       m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                       LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       !m_isDefault ? IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED
                                                    : IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT,
                                       RESOLVE_ATTACHMENT,
                                       ASPECT_COLOR,
                                       TEXTURE_2D);
        attachments[2].isDefault = m_isDefault ? true : false;

        attachments[3] = Graphics::AttachmentConfig(m_colorFormat,
                                                    1,
                                                    LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                    LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                    IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                                    RESOLVE_ATTACHMENT,
                                                    ASPECT_COLOR,
                                                    TEXTURE_2D,
                                                    FILTER_LINEAR,
                                                    ADDRESS_MODE_CLAMP_TO_EDGE);
    }

    Graphics::AttachmentConfig depthAttachment = Graphics::AttachmentConfig(m_depthFormat,
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
void ForwardPass::create_framebuffer() {
    CHECK_INITIALIZATION()

    if (m_aa == MSAASamples::x1) //IF NOT MULTISAMPLED
        for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
            m_framebuffers[fb] = m_device->create_framebuffer(
                m_renderpass, m_outAttachments, m_imageExtent, m_framebufferImageDepth, fb);

    else
    {
        std::vector<VKFW::Graphics::Image*> allAttachments{&m_interAttachments[0],
                                                           &m_interAttachments[1],
                                                           m_outAttachments[0],
                                                           m_outAttachments[1],
                                                           m_outAttachments[2]};
        for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
            m_framebuffers[fb] =
                m_device->create_framebuffer(m_renderpass, allAttachments, m_imageExtent, m_framebufferImageDepth, fb);
    }
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
        m_descriptorPool.update_descriptor(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                           sizeof(CameraUniforms),
                                           0,
                                           &m_descriptors[i].globalDescritor,
                                           UNIFORM_DYNAMIC_BUFFER,
                                           0);
        m_descriptorPool.update_descriptor(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                           sizeof(SceneUniforms),
                                           m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                           &m_descriptors[i].globalDescritor,
                                           UNIFORM_DYNAMIC_BUFFER,
                                           1);

        m_descriptorPool.update_descriptor(get_image(ResourceManager::FALLBACK_TEXTURE),
                                           LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                           &m_descriptors[i].globalDescritor,
                                           3);

        m_descriptorPool.update_descriptor(get_image(ResourceManager::textureResources[0]),
                                           LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                           &m_descriptors[i].globalDescritor,
                                           6);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.update_descriptor(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                           sizeof(ObjectUniforms),
                                           0,
                                           &m_descriptors[i].objectDescritor,
                                           UNIFORM_DYNAMIC_BUFFER,
                                           0);
        m_descriptorPool.update_descriptor(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                           sizeof(MaterialUniforms),
                                           m_device->pad_uniform_buffer_size(sizeof(MaterialUniforms)),
                                           &m_descriptors[i].objectDescritor,
                                           UNIFORM_DYNAMIC_BUFFER,
                                           1);
        // Set up enviroment fallback texture
        m_descriptorPool.update_descriptor(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                           LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                           &m_descriptors[i].globalDescritor,
                                           3);
        m_descriptorPool.update_descriptor(get_image(ResourceManager::FALLBACK_CUBEMAP),
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
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{
        Init::color_blend_attachment_state(true), Init::color_blend_attachment_state(true)};

    VkSampleCountFlagBits samples = static_cast<VkSampleCountFlagBits>(m_aa);

    // Setup shaderpasses
    GraphicShaderPass* unlitPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/forward/unlit.glsl");
    unlitPass->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, false}};
    unlitPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                   {NORMAL_ATTRIBUTE, false},
                                                   {UV_ATTRIBUTE, false},
                                                   {TANGENT_ATTRIBUTE, false},
                                                   {COLOR_ATTRIBUTE, false}};
    unlitPass->graphicSettings.blendAttachments = blendAttachments;
    unlitPass->graphicSettings.dynamicStates    = dynamicStates;
    unlitPass->graphicSettings.samples          = samples;
    m_shaderPasses["unlit"]                     = unlitPass;

    GraphicShaderPass* phongPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/forward/phong.glsl");
    phongPass->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    phongPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                   {NORMAL_ATTRIBUTE, true},
                                                   {UV_ATTRIBUTE, true},
                                                   {TANGENT_ATTRIBUTE, false},
                                                   {COLOR_ATTRIBUTE, false}};
    phongPass->graphicSettings.blendAttachments = blendAttachments;
    phongPass->graphicSettings.dynamicStates    = dynamicStates;
    phongPass->graphicSettings.samples          = samples;
    m_shaderPasses["phong"]                     = phongPass;

    GraphicShaderPass* PBRPass               = new GraphicShaderPass(m_device->get_handle(),
                                                       m_renderpass,
                                                       m_imageExtent,
                                                       ENGINE_RESOURCES_PATH "shaders/forward/physically_based.glsl");
    PBRPass->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    PBRPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                 {NORMAL_ATTRIBUTE, true},
                                                 {UV_ATTRIBUTE, true},
                                                 {TANGENT_ATTRIBUTE, true},
                                                 {COLOR_ATTRIBUTE, false}};
    PBRPass->graphicSettings.blendAttachments = blendAttachments;
    PBRPass->graphicSettings.dynamicStates    = dynamicStates;
    PBRPass->graphicSettings.samples          = samples;
    m_shaderPasses["physical"]                = PBRPass;

    GraphicShaderPass* hairStrandPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/forward/hair_strand.glsl");
    hairStrandPass->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, false}};
    hairStrandPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                        {NORMAL_ATTRIBUTE, false},
                                                        {UV_ATTRIBUTE, false},
                                                        {TANGENT_ATTRIBUTE, true},
                                                        {COLOR_ATTRIBUTE, true}};
    hairStrandPass->graphicSettings.dynamicStates    = dynamicStates;
    hairStrandPass->graphicSettings.blendAttachments = blendAttachments;
    hairStrandPass->graphicSettings.samples          = samples;
    hairStrandPass->graphicSettings.sampleShading    = false;
    hairStrandPass->graphicSettings.topology         = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    m_shaderPasses["hairstr"]                        = hairStrandPass;

    GraphicShaderPass* hairStrandPass2 = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/forward/hair_strand2.glsl");
    hairStrandPass2->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    hairStrandPass2->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                         {NORMAL_ATTRIBUTE, false},
                                                         {UV_ATTRIBUTE, false},
                                                         {TANGENT_ATTRIBUTE, true},
                                                         {COLOR_ATTRIBUTE, true}};
    hairStrandPass2->graphicSettings.dynamicStates    = dynamicStates;
    hairStrandPass2->graphicSettings.samples          = samples;
    hairStrandPass2->graphicSettings.sampleShading    = false;
    hairStrandPass2->graphicSettings.blendAttachments = blendAttachments;
    hairStrandPass2->graphicSettings.topology         = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    m_shaderPasses["hairstr2"]                        = hairStrandPass2;

    GraphicShaderPass* skyboxPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/forward/skybox.glsl");
    skyboxPass->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, false}, {OBJECT_TEXTURE_LAYOUT, false}};
    skyboxPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                    {NORMAL_ATTRIBUTE, false},
                                                    {UV_ATTRIBUTE, false},
                                                    {TANGENT_ATTRIBUTE, false},
                                                    {COLOR_ATTRIBUTE, false}};
    skyboxPass->graphicSettings.dynamicStates    = dynamicStates;
    skyboxPass->graphicSettings.samples          = samples;
    skyboxPass->graphicSettings.blendAttachments = blendAttachments;
    skyboxPass->graphicSettings.depthOp          = VK_COMPARE_OP_LESS_OR_EQUAL;
    m_shaderPasses["skybox"]                     = skyboxPass;

    for (auto pair : m_shaderPasses)
    {
        ShaderPass* pass = pair.second;

        pass->build_shader_stages();
        pass->build(m_descriptorPool);
    }
}

void ForwardPass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);

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

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);
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
            m_descriptorPool.update_descriptor(get_TLAS(scene), &m_descriptors[i].globalDescritor, 5);
        }
        get_TLAS(scene)->binded = true;
    }
}
void ForwardPass::link_input_attachments() {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        m_descriptorPool.update_descriptor(m_inAttachments[0],

                                           //   VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                           LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                           &m_descriptors[i].globalDescritor,
                                           2);
        m_descriptorPool.update_descriptor(
            m_inAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3);
        m_descriptorPool.update_descriptor(
            m_inAttachments[2], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 4);
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
                m_descriptorPool.update_descriptor(
                    get_image(texture), LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->get_texture_descriptor(), pair.first);
                mat->set_texture_binding_state(pair.first, true);
                texture->set_dirty(false);
            }
        } else
        {
            // SET DUMMY TEXTURE
            if (!mat->get_texture_binding_state()[pair.first])
                m_descriptorPool.update_descriptor(get_image(ResourceManager::FALLBACK_TEXTURE),
                                                   LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                   &mat->get_texture_descriptor(),
                                                   pair.first);
            mat->set_texture_binding_state(pair.first, true);
        }
    }
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END