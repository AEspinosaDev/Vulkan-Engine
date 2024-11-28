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

    if (!m_window->initialized())
        m_window->init();

    // Init Vulkan Device
    void* windowHandle{nullptr};
    m_window->get_handle(windowHandle);
    m_device.init(windowHandle,
                  m_window->get_windowing_system(),
                  m_window->get_extent(),
                  static_cast<uint32_t>(m_settings.bufferingType),
                  m_settings.colorFormat,
                  m_settings.screenSync);
    // Init resources
    init_resources();

    create_renderpasses();
    for (Core::BasePass* pass : m_passes)
    {
        if (pass->is_active())
        {
            pass->setup(m_frames);
            connect_renderpass(pass);
        }
    };

    m_deletionQueue.push_function([=]() {
        for (Core::BasePass* pass : m_passes)
        {
            pass->cleanup();
        }
    });

    if (m_settings.enableUI)
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
    m_device.wait();

    on_shutdown(scene);

    if (m_initialized)
    {
        m_deletionQueue.flush();

        clean_resources();
        Core::ResourceManager::clean_scene(scene);

        if (m_settings.enableUI)
            m_device.destroy_imgui();
        for (Core::BasePass* pass : m_passes)
        {
            pass->clean_framebuffer();
        }
        m_device.cleanup();
    }

    m_window->destroy();
    glfwTerminate();
}
void BaseRenderer::create_renderpasses() {
    throw VKFW_Exception("Implement setup_renderpasses function ! Hint: Add at least a forward pass ... ");
}
void BaseRenderer::on_before_render(Core::Scene* const scene) {
    PROFILING_EVENT()

    Core::ResourceManager::update_global_data(&m_device, &m_frames[m_currentFrame], scene, m_window);
    Core::ResourceManager::update_object_data(
        &m_device, &m_frames[m_currentFrame], scene, m_window, m_settings.enableRaytracing);

    for (Core::BasePass* pass : m_passes)
    {
        if (pass->is_active())
            pass->update_uniforms(m_currentFrame, scene);
    }
}

void BaseRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    PROFILING_EVENT()

    if (renderResult == RenderResult::ERROR_OUT_OF_DATE_KHR || renderResult == RenderResult::SUBOPTIMAL_KHR ||
        m_window->is_resized() || m_updateFramebuffers)
    {
        m_window->set_resized(false);
        update_renderpasses();
        scene->get_active_camera()->set_projection(m_window->get_extent().width, m_window->get_extent().height);
    } else if (renderResult != RenderResult::SUCCESS)
    { throw VKFW_Exception("failed to present swap chain image!"); }

    m_currentFrame = (m_currentFrame + 1) % m_frames.size();
}

void BaseRenderer::render(Core::Scene* const scene) {
    PROFILING_FRAME();
    PROFILING_EVENT()

    if (!m_initialized)
        init();

    uint32_t     imageIndex;
    RenderResult result = m_device.wait_frame(m_frames[m_currentFrame], imageIndex);

    if (result == RenderResult::ERROR_OUT_OF_DATE_KHR)
    {
        update_renderpasses();
        return;
    } else if (result != RenderResult::SUCCESS && result != RenderResult::SUBOPTIMAL_KHR)
    { throw VKFW_Exception("failed to acquire swap chain image!"); }

    on_before_render(scene);

    m_device.start_frame(m_frames[m_currentFrame]);

    if (scene->get_skybox())
        Core::ResourceManager::generate_skybox_maps(&m_frames[m_currentFrame], scene);

    for (Core::BasePass* pass : m_passes)
    {
        if (pass->is_active())
            pass->render(m_frames[m_currentFrame], scene, imageIndex);
    }

    RenderResult renderResult = m_device.submit_frame(m_frames[m_currentFrame], imageIndex);

    on_after_render(renderResult, scene);
}

void BaseRenderer::connect_renderpass(Core::BasePass* const currentPass) {
    if (currentPass->get_image_dependace_table().empty())
        return;

    std::vector<Graphics::Image> images;
    for (auto pair : currentPass->get_image_dependace_table())
    {
        std::vector<Graphics::Attachment> attachments = m_passes[pair.first]->get_attachments();
        for (size_t i = 0; i < pair.second.size(); i++)
        {
            images.push_back(attachments[pair.second[i]].image);
        }
    }
    currentPass->connect_to_previous_images(images);
}

void BaseRenderer::update_renderpasses() {

    m_window->update_framebuffer();
    m_device.wait();
    m_device.update_swapchain(m_window->get_extent(),
                              static_cast<uint32_t>(m_settings.bufferingType),
                              m_settings.colorFormat,
                              m_settings.screenSync);

    // Renderpass framebuffer updating
    for (Core::BasePass* pass : m_passes)
    {
        if (pass->is_active())
        {
            if (pass->resizeable())
            {
                pass->set_extent(m_window->get_extent());
                pass->update();
            }
            connect_renderpass(pass);
        }
    };

    m_updateFramebuffers = false;
}

void BaseRenderer::init_gui() {

    if (m_settings.enableUI)
    {
        // Look for default pass
        Core::BasePass* defaultPass = nullptr;
        for (Core::BasePass* pass : m_passes)
        {
            if (pass->is_active() && pass->default_pass())
            {
                defaultPass = pass;
            }
        };

        void* windowHandle;
        m_window->get_handle(windowHandle);
        m_device.init_imgui(windowHandle,
                            m_window->get_windowing_system(),
                            defaultPass->get_handle(),
                            static_cast<uint16_t>(m_settings.samplesMSAA));
    }
}
void BaseRenderer::init_resources() {

    // Setup frames
    m_frames.resize(static_cast<uint32_t>(m_settings.bufferingType));
    for (size_t i = 0; i < m_frames.size(); i++)
        m_frames[i] = m_device.create_frame(i);
    for (size_t i = 0; i < m_frames.size(); i++)
    {
        // Global Buffer
        const size_t     globalStrideSize = (m_device.pad_uniform_buffer_size(sizeof(Graphics::CameraUniforms)) +
                                         m_device.pad_uniform_buffer_size(sizeof(Graphics::SceneUniforms)));
        Graphics::Buffer globalBuffer     = m_device.create_buffer_VMA(
            globalStrideSize, BUFFER_USAGE_UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)globalStrideSize);
        m_frames[i].uniformBuffers.push_back(globalBuffer);

        // Object Buffer
        const size_t     objectStrideSize = (m_device.pad_uniform_buffer_size(sizeof(Graphics::ObjectUniforms)) +
                                         m_device.pad_uniform_buffer_size(sizeof(Graphics::MaterialUniforms)));
        Graphics::Buffer objectBuffer     = m_device.create_buffer_VMA(ENGINE_MAX_OBJECTS * objectStrideSize,
                                                                   BUFFER_USAGE_UNIFORM_BUFFER,
                                                                   VMA_MEMORY_USAGE_CPU_TO_GPU,
                                                                   (uint32_t)objectStrideSize);
        m_frames[i].uniformBuffers.push_back(objectBuffer);
    }
    Core::ResourceManager::init_basic_resources(&m_device);
}

void BaseRenderer::clean_resources() {
    for (size_t i = 0; i < m_frames.size(); i++)
    {
        m_frames[i].cleanup();
    }
    Core::ResourceManager::clean_basic_resources();
}
} // namespace Systems

VULKAN_ENGINE_NAMESPACE_END