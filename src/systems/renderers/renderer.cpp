/*
        This file is part of Vulkan-Engine, a simple to use Vulkan based 3D
   library

        MIT License

        Copyright (c) 2023 Antonio Espinosa Garcia

*/
#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {
void BaseRenderer::init() {

    // Init Vulkan Device
    m_device = std::make_shared<Graphics::Device>();

    if (!m_headless)
    {
        if (!m_window->initialized())
            m_window->init();
        void* windowHandle{nullptr};
        m_window->get_handle(windowHandle);
        m_device->init(windowHandle,
                       m_window->get_windowing_system(),
                       m_window->get_extent(),
                       static_cast<uint32_t>(m_settings.bufferingType),
                       m_settings.displayColorFormat,
                       m_settings.screenSync);
    } else
    {
        m_device->init_headless();
    }

    // Init resources
    init_resources();
    // User defined renderpasses
    create_passes();
    // Init renderpasses
    for (auto& pass : m_passes)
        if (pass->is_active())
            pass->setup(m_frames);
    // Connect renderpasses
    for (auto& pass : m_passes)
        if (pass->is_active())
            pass->link_input_attachments();

    if (m_settings.enableUI && !m_headless)
        init_gui();

    Graphics::Frame::guiEnabled = m_settings.enableUI;

    m_initialized = true;
}
void BaseRenderer::run(Core::Scene* const scene) {
    ASSERT_PTR(m_window);
    while (!m_window->get_window_should_close())
    {
        // I-O
        PROFILING_FRAME();
        m_window->poll_events();
        render(scene);
    }
    shutdown(scene);
}

void BaseRenderer::shutdown(Core::Scene* const scene) {
    m_device->wait_idle();

    on_shutdown(scene);

    if (m_initialized)
    {
        m_deletionQueue.flush();
        for (auto& pass : m_passes)
        {
            pass->cleanup();
        }

        clean_resources();
        Core::ResourceManager::clean_scene(scene);

        if (m_settings.enableUI && !m_headless)
            m_device->destroy_imgui();

        m_device->cleanup();
    }

    if (!m_headless)
        m_window->destroy();

    glfwTerminate();
}
void BaseRenderer::create_passes() {

    throw VKFW_Exception("Implement setup_renderpasses function ! Hint: Add at least a forward pass ... ");
}
void BaseRenderer::on_before_render(Core::Scene* const scene) {
    PROFILING_EVENT()

    const Extent2D DISPLAY_EXTENT = !m_headless ? m_window->get_extent() : m_headlessExtent;
    Core::ResourceManager::update_global_data(m_device, &m_frames[m_currentFrame], scene, DISPLAY_EXTENT, m_settings.softwareAA == SoftwareAA::TAA);
    Core::ResourceManager::update_object_data(m_device, &m_frames[m_currentFrame], scene, DISPLAY_EXTENT, m_settings.enableRaytracing);

    for (auto& pass : m_passes)
    {
        if (pass->is_active())
            pass->update_uniforms(m_currentFrame, scene);
    }
}

void BaseRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    PROFILING_EVENT()

    // Save old camera state
    auto camera                         = scene->get_active_camera();
    Core::ResourceManager::prevViewProj = camera->get_projection() * camera->get_view();

    if (!m_headless)
    {
        if (renderResult == RenderResult::ERROR_OUT_OF_DATE_KHR || renderResult == RenderResult::SUBOPTIMAL_KHR || m_window->is_resized() ||
            m_updateFramebuffers)
        {
            m_window->set_resized(false);
            m_window->update_framebuffer();
            update_framebuffers(m_window->get_extent());
            scene->get_active_camera()->set_projection(m_window->get_extent().width, m_window->get_extent().height);
        } else if (renderResult != RenderResult::SUCCESS)
        { throw VKFW_Exception("failed to present swap chain image!"); }
    }
    m_currentFrame = (m_currentFrame + 1) % m_frames.size();
}

void BaseRenderer::render(Core::Scene* const scene) {
    PROFILING_FRAME();
    PROFILING_EVENT()

    if (!m_initialized)
        init();

    if (!scene->get_active_camera())
        return;

    uint32_t imageIndex = 0;
    if (!m_headless)
    {
        RenderResult result = m_device->wait_frame(m_frames[m_currentFrame], imageIndex);

        if (result == RenderResult::ERROR_OUT_OF_DATE_KHR)
        {
            m_window->update_framebuffer();
            update_framebuffers(m_window->get_extent());
            return;
        } else if (result != RenderResult::SUCCESS && result != RenderResult::SUBOPTIMAL_KHR)
        { throw VKFW_Exception("failed to acquire swap chain image!"); }

    } else
    {
        m_frames[m_currentFrame].renderFence.wait();
    }

    on_before_render(scene);

    m_device->start_frame(m_frames[m_currentFrame]);

    for (auto& pass : m_passes)
    {
        if (pass->is_active())
            pass->execute(m_frames[m_currentFrame], scene, imageIndex);
    }

    RenderResult renderResult = RenderResult::SUCCESS;
    if (!m_headless)
        renderResult = m_device->submit_frame(m_frames[m_currentFrame], imageIndex);
    else
    {
        m_frames[m_currentFrame].commandBuffer.end();
        m_frames[m_currentFrame].commandBuffer.submit(m_frames[m_currentFrame].renderFence);
    }

    on_after_render(renderResult, scene);
}

void BaseRenderer::update_framebuffers(Extent2D extent) {

    m_device->wait_idle();
    m_device->update_swapchain(extent, static_cast<uint32_t>(m_settings.bufferingType), m_settings.displayColorFormat, m_settings.screenSync);

    // Renderpass framebuffer updating
    for (auto& pass : m_passes)
    {
        if (pass->is_active())
            if (pass->resizeable())
            {
                pass->set_extent(extent);
                pass->resize_attachments();
            }
    };
    // Connect renderpasses
    for (auto& pass : m_passes)
        if (pass->is_active())
            pass->link_input_attachments();

    m_updateFramebuffers = false;
}

void BaseRenderer::init_gui() {

    if (m_settings.enableUI)
    {
        // Look for default pass
        ptr<Core::BasePass> defaultPass = nullptr;
        for (auto& pass : m_passes)
        {
            if (pass->is_active() && pass->default_pass())
            {
                defaultPass = pass;
            }
        };

        void* windowHandle;
        m_window->get_handle(windowHandle);
        m_device->init_imgui(windowHandle,
                             m_window->get_windowing_system(),
                             std::static_pointer_cast<Core::BaseGraphicPass>(defaultPass)->get_renderpass(),
                             std::static_pointer_cast<Core::BaseGraphicPass>(defaultPass)->get_renderpass().attachmentsConfig[0].imageConfig.samples);
    }
}

void BaseRenderer::init_resources() {

    // Setup frames
    m_frames.resize(static_cast<uint32_t>(m_settings.bufferingType));
    for (size_t i = 0; i < m_frames.size(); i++)
        m_frames[i] = m_device->create_frame(i);
    for (size_t i = 0; i < m_frames.size(); i++)
    {
        // Global Buffer
        const size_t globalStrideSize =
            (m_device->pad_uniform_buffer_size(sizeof(Graphics::CameraUniforms)) + m_device->pad_uniform_buffer_size(sizeof(Graphics::SceneUniforms)));
        Graphics::Buffer globalBuffer =
            m_device->create_buffer_VMA(globalStrideSize, BUFFER_USAGE_UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)globalStrideSize);
        m_frames[i].uniformBuffers.push_back(globalBuffer);

        // Object Buffer
        const size_t objectStrideSize =
            (m_device->pad_uniform_buffer_size(sizeof(Graphics::ObjectUniforms)) + m_device->pad_uniform_buffer_size(sizeof(Graphics::MaterialUniforms)));
        Graphics::Buffer objectBuffer = m_device->create_buffer_VMA(
            ENGINE_MAX_OBJECTS * objectStrideSize, BUFFER_USAGE_UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)objectStrideSize);
        m_frames[i].uniformBuffers.push_back(objectBuffer);
    }

    Core::ResourceManager::init_basic_resources(m_device);

    // Create basic texture resources
    Core::ResourceManager::textureResources.resize(2, nullptr);

    Core::TextureLDR* samplerText = new Core::TextureLDR();
    Tools::Loaders::load_PNG(samplerText, GET_RESOURCE_PATH("textures/blueNoise.png"), TEXTURE_FORMAT_UNORM);
    samplerText->set_use_mipmaps(false);
    Core::ResourceManager::upload_texture_data(m_device, samplerText);
    Core::ResourceManager::textureResources[0] = samplerText;

    Core::TextureHDR* brdfText = new Core::TextureHDR();
    Tools::Loaders::load_HDRi(brdfText, GET_RESOURCE_PATH("textures/cookTorranceBRDF.png"));
    brdfText->set_adress_mode(ADDRESS_MODE_CLAMP_TO_BORDER);
    brdfText->set_use_mipmaps(false);
    Core::ResourceManager::upload_texture_data(m_device, brdfText);
    Core::ResourceManager::textureResources[1] = brdfText;
}

void BaseRenderer::clean_resources() {
    for (size_t i = 0; i < m_frames.size(); i++)
        m_frames[i].cleanup();
    for (size_t i = 0; i < m_attachments.size(); i++)
        m_attachments[i].cleanup();
    Core::ResourceManager::clean_basic_resources();
}

Core::ITexture* BaseRenderer::capture_texture(uint32_t attachmentId) {
    void*  imageData     = nullptr;
    size_t imageSize     = 0;
    size_t imageChannels = 0;
    m_device->download_texture_image(m_attachments[attachmentId], imageData, imageSize, imageChannels);

    Core::TextureSettings settings = {};
    settings.format                = m_attachments[attachmentId].config.format;
    settings.type                  = m_attachments[attachmentId].config.viewType;
    settings.useMipmaps            = false;

    Core::ITexture* tex = nullptr;
    if (Utils::is_hdr_format(settings.format))
        tex = new Core::TextureHDR(reinterpret_cast<float*>(imageData), m_attachments[attachmentId].extent, imageChannels, settings);
    else
        tex = new Core::TextureLDR(reinterpret_cast<unsigned char*>(imageData), m_attachments[attachmentId].extent, imageChannels, settings);
    return tex;
}

} // namespace Systems

VULKAN_ENGINE_NAMESPACE_END