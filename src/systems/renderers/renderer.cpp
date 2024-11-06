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

    //Init Vulkan Device
    void* windowHandle{nullptr};
    m_window->get_handle(windowHandle);
    m_device.init(windowHandle,
                  m_window->get_windowing_system(),
                  m_window->get_extent(),
                  static_cast<uint32_t>(m_settings.bufferingType),
                  static_cast<VkFormat>(m_settings.colorFormat),
                  static_cast<VkPresentModeKHR>(m_settings.screenSync));
    //Init resources
    init_resources();

    setup_renderpasses();
    for (Core::RenderPass* pass : m_renderPipeline.renderpasses)
    {
        if (pass->is_active())
        {
            pass->setup();
            connect_renderpass(pass);
        }
    };

    m_deletionQueue.push_function([=]() { m_renderPipeline.flush(); });

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

        clean_Resources();

        if (scene)
            for (Core::Mesh* m : scene->get_meshes())
            {
                for (size_t i = 0; i < m->get_num_geometries(); i++)
                {
                    Core::Geometry* g = m->get_geometry(i);
                    destroy_geometry_data(g);
                    Core::IMaterial* mat = m->get_material(g->get_material_ID());
                    if (mat)
                    {
                        auto textures = mat->get_textures();
                        for (auto pair : textures)
                        {
                            Core::ITexture* texture = pair.second;
                            destroy_texture_image(texture);
                        }
                    }
                }
            }

        if (scene->get_skybox())
        {
            destroy_geometry_data(scene->get_skybox()->get_box());
            destroy_texture_image(scene->get_skybox()->get_enviroment_map());
        }

        m_renderPipeline.flush_framebuffers();
        for (size_t i = 0; i < m_frames.size(); i++)
        {
            m_frames[i].cleanup();
        }
        m_device.cleanup();
    }

    m_window->destroy();

    glfwTerminate();
}
void BaseRenderer::setup_renderpasses() {
    throw VKException("Implement setup_renderpasses function ! Hint: Add at least a forward pass ... ");
}
void BaseRenderer::on_before_render(Core::Scene* const scene) {
    PROFILING_EVENT()

    update_global_data(scene);
    update_object_data(scene);

    for (Core::RenderPass* pass : m_renderPipeline.renderpasses)
    {
        if (pass->is_active())
            pass->update_uniforms(m_currentFrame, scene);
    }
}

void BaseRenderer::on_after_render(VkResult& renderResult, Core::Scene* const scene) {
    PROFILING_EVENT()

    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR || m_window->is_resized() ||
        m_updateFramebuffers)
    {
        m_window->set_resized(false);
        update_renderpasses();
        scene->get_active_camera()->set_projection(m_window->get_extent().width, m_window->get_extent().height);
    } else if (renderResult != VK_SUCCESS)
    { throw VKException("failed to present swap chain image!"); }

    m_currentFrame = (m_currentFrame + 1) % m_frames.size();
}

void BaseRenderer::render(Core::Scene* const scene) {
    PROFILING_FRAME();
    PROFILING_EVENT()

    if (!m_initialized)
        init();

    uint32_t imageIndex;
    VkResult imageResult = m_device.aquire_present_image(m_frames[m_currentFrame], imageIndex);

    if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        update_renderpasses();
        return;
    } else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
    { throw VKException("failed to acquire swap chain image!"); }

    on_before_render(scene);

    m_device.begin_command_buffer(m_frames[m_currentFrame]);

    if (scene->get_skybox())
        if (scene->get_skybox()->update_enviroment())
        {
            m_renderPipeline.panoramaConverterPass->render(m_currentFrame, scene, imageIndex);
            m_renderPipeline.irradianceComputePass->render(m_currentFrame, scene, imageIndex);
            scene->get_skybox()->set_update_enviroment(false);
        }

    m_renderPipeline.render(m_currentFrame, scene, imageIndex);

    m_device.end_command_buffer(m_frames[m_currentFrame]);

    VkResult renderResult = m_device.present_image(m_frames[m_currentFrame], imageIndex);

    on_after_render(renderResult, scene);
}

void BaseRenderer::connect_renderpass(Core::RenderPass* const currentPass) {
    if (currentPass->get_image_dependace_table().empty())
        return;

    std::vector<Graphics::Image> images;
    for (auto pair : currentPass->get_image_dependace_table())
    {
        std::vector<Core::Attachment> attachments = m_renderPipeline.renderpasses[pair.first]->get_attachments();
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
                              static_cast<VkFormat>(m_settings.colorFormat),
                              static_cast<VkPresentModeKHR>(m_settings.screenSync));

    // Renderpass framebuffer updating
    for (Core::RenderPass* pass : m_renderPipeline.renderpasses)
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
        Core::RenderPass* defaultPass = nullptr;
        for (Core::RenderPass* pass : m_renderPipeline.renderpasses)
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
                            static_cast<VkSampleCountFlagBits>(m_settings.samplesMSAA));

        m_deletionQueue.push_function([=]() { m_device.destroy_imgui(); });
    }
}

} // namespace Systems

VULKAN_ENGINE_NAMESPACE_END