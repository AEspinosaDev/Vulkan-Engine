#include <engine/core/passes/voxelization_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void VoxelizationPass::create_voxelization_image() {

    // Actual Voxel Image
    m_outAttachments[0]->cleanup();

    ImageConfig config   = {};
    config.viewType      = TEXTURE_3D;
    config.format        = SRGBA_32F;
    config.usageFlags    = IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_DST | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_STORAGE;
    config.mipLevels     = 9;
    *m_outAttachments[0] = m_device->create_image({m_imageExtent.width, m_imageExtent.width, m_imageExtent.width}, config);
    m_outAttachments[0]->create_view(config);

    SamplerConfig samplerConfig      = {};
    samplerConfig.samplerAddressMode = ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerConfig.border             = BorderColor::FLOAT_OPAQUE_BLACK;
    m_outAttachments[0]->create_sampler(samplerConfig);

#ifdef USE_IMG_ATOMIC_OPERATION
    // Auxiliar One Channel Images
    config.format            = R_32_UINT;
    config.mipLevels         = 1;
    samplerConfig.filters    = FilterType::FILTER_NEAREST;
    samplerConfig.mipmapMode = MipmapMode::MIPMAP_NEAREST;
    m_interAttachments.resize(4);
    for (size_t i = 0; i < 3; i++)
    {

        m_interAttachments[i].cleanup();
        m_interAttachments[i] = m_device->create_image({m_imageExtent.width, m_imageExtent.width, m_imageExtent.width}, config);
        m_interAttachments[i].create_view(config);
        m_interAttachments[i].create_sampler(samplerConfig);
    }
#endif
}

void VoxelizationPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) {

    // m_imageExtent = {1, 1};
    attachments.resize(1);

    attachments[0] = Graphics::AttachmentConfig(RGBA_8U,
                                                1,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                                COLOR_ATTACHMENT,
                                                ASPECT_COLOR);
    create_voxelization_image();
    // Depdencies
    dependencies.resize(2);

    dependencies[0]               = Graphics::SubPassDependency(STAGE_BOTTOM_OF_PIPE, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[0].srcAccessMask = ACCESS_MEMORY_READ;
    dependencies[1]               = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_BOTTOM_OF_PIPE, ACCESS_MEMORY_READ);
    dependencies[1].srcAccessMask = ACCESS_COLOR_ATTACHMENT_WRITE;
    dependencies[1].srcSubpass    = 0;
    dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;

    m_isResizeable = false;
}
void VoxelizationPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    m_descriptorPool = m_device->create_descriptor_pool(ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding  camBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding  sceneBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding  shadowBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding  iblBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding  accelBinding(UNIFORM_ACCELERATION_STRUCTURE, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding  noiseBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5);
    LayoutBinding  voxelBinding(UNIFORM_STORAGE_IMAGE, SHADER_STAGE_FRAGMENT, 6);
    const uint32_t RGB_CHANNELS = 3;
    LayoutBinding  auxVoxelBinding(UNIFORM_STORAGE_IMAGE, SHADER_STAGE_FRAGMENT, 7, RGB_CHANNELS);
    LayoutBinding  auxVoxelBindingSampler(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 8, RGB_CHANNELS);
    m_descriptorPool.set_layout(
        GLOBAL_LAYOUT,
        {camBufferBinding, sceneBufferBinding, shadowBinding, iblBinding, accelBinding, noiseBinding, voxelBinding, auxVoxelBinding, auxVoxelBindingSampler});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding materialBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    // MATERIAL TEXTURE SET
    // MATERIAL TEXTURE SET
    LayoutBinding materialTextureBufferBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0, MAX_TEXTURES);
    m_descriptorPool.set_layout(OBJECT_TEXTURE_LAYOUT,
                                {materialTextureBufferBinding},
                                0,
                                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptors[i].globalDescritor.update(&frames[i].uniformBuffers[GLOBAL_LAYOUT], sizeof(CameraUniforms), 0, UNIFORM_DYNAMIC_BUFFER, 0);
        m_descriptors[i].globalDescritor.update(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                                sizeof(SceneUniforms),
                                                m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),

                                                UNIFORM_DYNAMIC_BUFFER,
                                                1);

        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::FALLBACK_TEXTURE), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 3);

        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::textureResources[0]), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 5);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptors[i].objectDescritor.update(&frames[i].uniformBuffers[OBJECT_LAYOUT], sizeof(ObjectUniforms), 0, UNIFORM_DYNAMIC_BUFFER, 0);
        m_descriptors[i].objectDescritor.update(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                                sizeof(MaterialUniforms),
                                                m_device->pad_uniform_buffer_size(sizeof(MaterialUniforms)),
                                                UNIFORM_DYNAMIC_BUFFER,
                                                1);
        // Set up enviroment fallback texture
        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::FALLBACK_CUBEMAP), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 3);
        // Voxelization Image
        m_descriptors[i].globalDescritor.update(m_outAttachments[0], LAYOUT_GENERAL, 6, UNIFORM_STORAGE_IMAGE);
#ifdef USE_IMG_ATOMIC_OPERATION
        // Voxelization Aux.Images
        std::vector<Graphics::Image> auxImages = {m_interAttachments[0], m_interAttachments[1], m_interAttachments[2]};
        m_descriptors[i].globalDescritor.update(auxImages, LAYOUT_GENERAL, 7, UNIFORM_STORAGE_IMAGE);
        m_descriptors[i].globalDescritor.update(auxImages, LAYOUT_GENERAL, 8);
#endif

        // Textures
        m_descriptorPool.allocate_variable_descriptor_set(OBJECT_TEXTURE_LAYOUT, &m_descriptors[i].textureDescritor, MAX_TEXTURES);
    }
}
void VoxelizationPass::setup_shader_passes() {

    GraphicShaderPass* voxelPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/VXGI/voxelization.glsl"));
    voxelPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    voxelPass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, true}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, false}, {COLOR_ATTRIBUTE, false}};
    voxelPass->graphicSettings.dynamicStates    = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineColorBlendAttachmentState state   = Init::color_blend_attachment_state(false);
    state.colorWriteMask                        = 0;
    voxelPass->graphicSettings.blendAttachments = {state};

    voxelPass->build_shader_stages();
    voxelPass->build(m_descriptorPool);

    m_shaderPasses["voxelization"] = voxelPass;

#ifdef USE_IMG_ATOMIC_OPERATION

    ComputeShaderPass* mergePass               = new ComputeShaderPass(m_device->get_handle(), GET_RESOURCE_PATH("shaders/VXGI/merge_intermediates.glsl"));
    mergePass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, false}, {OBJECT_TEXTURE_LAYOUT, false}};

    mergePass->build_shader_stages();
    mergePass->build(m_descriptorPool);

    m_shaderPasses["merge"] = mergePass;

#endif
}
void VoxelizationPass::link_input_attachments() {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        // Shadows
        m_descriptors[i].globalDescritor.update(m_inAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
    }
}
void VoxelizationPass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;

    /*
    PREPARE VOXEL IMAGES TO BE USED IN SHADERS
    */
    if (m_outAttachments[0]->currentLayout == LAYOUT_UNDEFINED)
    {

        for (Graphics::Image* img : m_outAttachments)
            cmd.pipeline_barrier(*img, LAYOUT_UNDEFINED, LAYOUT_GENERAL, ACCESS_NONE, ACCESS_SHADER_READ, STAGE_TOP_OF_PIPE, STAGE_FRAGMENT_SHADER);
        for (Graphics::Image& img : m_interAttachments)
            cmd.pipeline_barrier(img, LAYOUT_UNDEFINED, LAYOUT_GENERAL, ACCESS_NONE, ACCESS_SHADER_READ, STAGE_TOP_OF_PIPE, STAGE_FRAGMENT_SHADER);
    }

    /*
    CLEAR IMAGES
    */
    for (Graphics::Image* img : m_outAttachments)
        cmd.clear_image(*img, LAYOUT_GENERAL, ASPECT_COLOR, Vec4(0.0));
    for (size_t i = 0; i < 3; i++)
    {
        cmd.clear_image(m_interAttachments[i], LAYOUT_GENERAL, ASPECT_COLOR, Vec4(0.0));
        cmd.pipeline_barrier(
            m_interAttachments[i], LAYOUT_UNDEFINED, LAYOUT_GENERAL, ACCESS_NONE, ACCESS_SHADER_READ, STAGE_TOP_OF_PIPE, STAGE_FRAGMENT_SHADER);
    }

    /*
    POPULATE AUXILIAR IMAGES WITH DIRECT IRRADIANCE
    */
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);

    cmd.set_viewport(m_imageExtent);

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {

        ShaderPass* shaderPass = m_shaderPasses["voxelization"];
        // Bind pipeline
        cmd.bind_shaderpass(*shaderPass);
        // GLOBAL LAYOUT BINDING
        cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
        // TEXTURE LAYOUT BINDING
        if (shaderPass->settings.descriptorSetLayoutIDs[OBJECT_TEXTURE_LAYOUT])
            cmd.bind_descriptor_set(m_descriptors[currentFrame.index].textureDescritor, 2, *shaderPass);

        unsigned int mesh_idx = 0;
        for (Mesh* m : scene->get_meshes())
        {
            if (m)
            {
                if (m->is_active() &&  // Check if is active
                    m->get_geometry()) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset = currentFrame.uniformBuffers[1].strideSize * mesh_idx;

                    auto g   = m->get_geometry();
                    auto mat = m->get_material();

                    // PER OBJECT LAYOUT BINDING
                    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].objectDescritor, 1, *shaderPass, {objectOffset, objectOffset});

                    // DRAW
                    cmd.draw_geometry(*get_VAO(g));
                }
            }
            mesh_idx++;
        }
    }

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);

    /*
    DISPATCH COMPUTE FOR POPULATING FINAL IMAGE WITH CONTENT OF AUX.IMAGES
    */
#ifdef USE_IMG_ATOMIC_OPERATION

    ShaderPass* mergePass = m_shaderPasses["merge"];
    cmd.bind_shaderpass(*mergePass);

    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *mergePass, {0, 0}, BINDING_TYPE_COMPUTE);

    // Dispatch the compute shader
    const uint32_t WORK_GROUP_SIZE = 4;
    uint32_t       gridSize        = std::max(1u, m_imageExtent.width);
    gridSize                       = (gridSize + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
    cmd.dispatch_compute({gridSize, gridSize, gridSize});

#endif

    /*
       GENERATE MIPMAPS FOR UPPER IRRADIANCE LEVELS
       */
    cmd.pipeline_barrier(
        *m_outAttachments[0], LAYOUT_GENERAL, LAYOUT_TRANSFER_DST_OPTIMAL, ACCESS_SHADER_WRITE, ACCESS_TRANSFER_READ, STAGE_COMPUTE_SHADER, STAGE_TRANSFER);

    cmd.generate_mipmaps(*m_outAttachments[0], LAYOUT_TRANSFER_DST_OPTIMAL, LAYOUT_GENERAL);
}

void VoxelizationPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    uint32_t meshIdx = 0;
    for (Mesh* m : scene->get_meshes())
    {
        if (m)
        {
            auto g   = m->get_geometry();
            auto mat = m->get_material();
            setup_material_descriptor(mat, meshIdx);
        }
        meshIdx++;
    }

    for (size_t i = 0; i < m_descriptors.size(); i++)
    {

        m_descriptors[i].globalDescritor.update(get_TLAS(scene), 4);
    }
}
void VoxelizationPass::resize_attachments() {
    for (Graphics::Framebuffer& fb : m_framebuffers)
        fb.cleanup();
    for (Graphics::Image* img : m_outAttachments)
        img->cleanup();
    for (Graphics::Image& img : m_interAttachments)
        img.cleanup();
    create_voxelization_image();
    create_framebuffer();

    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        // Voxelization Image
        m_descriptors[i].globalDescritor.update(m_outAttachments[0], LAYOUT_GENERAL, 6, UNIFORM_STORAGE_IMAGE);
#ifdef USE_IMG_ATOMIC_OPERATION
        // Voxelization Aux.Images
        std::vector<Graphics::Image> auxImages = {m_interAttachments[0], m_interAttachments[1], m_interAttachments[2]};
        m_descriptors[i].globalDescritor.update(auxImages, LAYOUT_GENERAL, 7, UNIFORM_STORAGE_IMAGE);
        m_descriptors[i].globalDescritor.update(auxImages, LAYOUT_GENERAL, 8);
#endif
    }
}
void VoxelizationPass::setup_material_descriptor(IMaterial* mat, uint32_t meshIdx) {
    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        ITexture* texture = pair.second;
        if (texture && texture->loaded_on_GPU())
        {
            for (size_t i = 0; i < m_descriptors.size(); i++)
            {
                uint32_t bindingPoint = 0;
                m_descriptors[i].textureDescritor.update(
                    get_image(texture), LAYOUT_SHADER_READ_ONLY_OPTIMAL, bindingPoint, UNIFORM_COMBINED_IMAGE_SAMPLER, pair.first + 6 * meshIdx);
            }
        }
    }
}

void VoxelizationPass::create_framebuffer() {
    std::vector<Graphics::Image*> out = {&m_interAttachments[3]};
    m_framebuffers[0]                 = m_device->create_framebuffer(m_renderpass, out, m_imageExtent, m_framebufferImageDepth, 0);
}

void VoxelizationPass::cleanup() {
    for (Graphics::Image* img : m_outAttachments)
        img->cleanup();

    for (size_t i = 0; i < m_interAttachments.size(); i++)
    {
        m_interAttachments[i].cleanup();
    }
    BaseGraphicPass::cleanup();
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
