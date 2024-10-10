/*
        This file is part of Vulkan-Engine, a simple to use Vulkan based 3D
   library

        MIT License

        Copyright (c) 2023 Antonio Espinosa Garcia

*/
#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::init()
{

    if (!m_window->initialized())
        m_window->init();

    m_context.init(m_window->get_handle(), m_window->get_extent(), static_cast<uint32_t>(m_settings.bufferingType),
                   static_cast<VkFormat>(m_settings.colorFormat), static_cast<VkPresentModeKHR>(m_settings.screenSync));

    init_resources();
    setup_renderpasses();

    for (RenderPass *pass : m_renderPipeline.renderpasses)
    {
        if (pass->is_active())
        {
            pass->init();
            pass->create_framebuffer();
            pass->create_descriptors();
            pass->create_graphic_pipelines();
            pass->init_resources();
            connect_renderpass(pass);
        }
    };

    m_deletionQueue.push_function([=]() {
        for (RenderPass *pass : m_renderPipeline.renderpasses)
        {
            pass->cleanup();
        }
    });

    if (m_settings.enableUI)
        init_gui();

    Frame::guiEnabled = m_settings.enableUI;

    m_initialized = true;
}
void Renderer::run(Scene *const scene)
{
    while (!glfwWindowShouldClose(m_window->get_handle()))
    {
        // I-O
        Window::poll_events();
        render(scene);
    }
    shutdown(scene);
}

void Renderer::shutdown(Scene *const scene)
{
    m_context.wait_for_device();

    on_shutdown(scene);

    if (m_initialized)
    {
        m_deletionQueue.flush();

        clean_Resources();

        if (scene)
            for (Mesh *m : scene->get_meshes())
            {
                for (size_t i = 0; i < m->get_num_geometries(); i++)
                {
                    Geometry *g = m->get_geometry(i);
                    RenderData rd = g->get_render_data();
                    if (rd.loaded)
                    {
                        rd.vbo.cleanup(m_context.memory);
                        if (g->indexed())
                            rd.ibo.cleanup(m_context.memory);

                        rd.loaded = false;
                        g->set_render_data(rd);
                    }
                }
            }

        for (RenderPass *pass : m_renderPipeline.renderpasses)
        {
            pass->clean_framebuffer();
        }

        m_context.cleanup();
    }

    m_window->destroy();

    glfwTerminate();
}

void Renderer::on_before_render(Scene *const scene)
{
    if (Frame::guiEnabled && m_gui)
        m_gui->render();

    update_global_data(scene);
    update_object_data(scene);

    for (RenderPass *pass : m_renderPipeline.renderpasses)
    {
        if (pass->is_active())
            pass->upload_data(m_currentFrame, scene);
    }
}

void Renderer::on_after_render(VkResult &renderResult, Scene *const scene)
{
    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR || m_window->is_resized() ||
        m_updateFramebuffers)
    {
        m_window->set_resized(false);
        update_renderpasses();
        scene->get_active_camera()->set_projection(m_window->get_extent().width, m_window->get_extent().height);
    }
    else if (renderResult != VK_SUCCESS)
    {
        throw VKException("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % m_context.frames.size();
}

void Renderer::render(Scene *const scene)
{
    if (!m_initialized)
        init();

    uint32_t imageIndex;
    VkResult imageResult = m_context.aquire_present_image(m_currentFrame, imageIndex);

    if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        update_renderpasses();
        return;
    }
    else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
    {
        throw VKException("failed to acquire swap chain image!");
    }

    on_before_render(scene);

    m_context.begin_command_buffer(m_currentFrame);

    for (RenderPass *pass : m_renderPipeline.renderpasses)
    {
        if (pass->is_active())
            pass->render(m_currentFrame, scene, imageIndex);
    }

    m_context.end_command_buffer(m_currentFrame);

    VkResult renderResult = m_context.present_image(m_currentFrame, imageIndex);

    on_after_render(renderResult, scene);
}

void Renderer::connect_renderpass(RenderPass *const currentPass)
{
    if (currentPass->get_image_dependace_table().empty())
        return;

    std::vector<Image> images;
    for (auto pair : currentPass->get_image_dependace_table())
    {
        std::vector<Attachment> attachments = m_renderPipeline.renderpasses[pair.first]->get_attachments();
        for (size_t i = 0; i < pair.second.size(); i++)
        {
            images.push_back(attachments[pair.second[i]].image);
        }
    }
    currentPass->connect_to_previous_images(images);
}

void Renderer::update_renderpasses()
{
    // GLFW update framebuffer
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window->get_handle(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_window->get_handle(), &width, &height);
        glfwWaitEvents();
    }

    m_context.wait_for_device();

    m_context.recreate_swapchain(
        m_window->get_handle(), m_window->get_extent(), static_cast<uint32_t>(m_settings.bufferingType),
        static_cast<VkFormat>(m_settings.colorFormat), static_cast<VkPresentModeKHR>(m_settings.screenSync));

    // Renderpass framebuffer updating
    for (RenderPass *pass : m_renderPipeline.renderpasses)
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

void Renderer::init_gui()
{
    if (m_gui)
    {
        // Look for default pass
        RenderPass *defaultPass = nullptr;
        for (RenderPass *pass : m_renderPipeline.renderpasses)
        {
            if (pass->is_active() && pass->default_pass())
            {
                defaultPass = pass;
            }
        };

        m_gui->init(m_context, defaultPass->get_handle(), m_window->get_handle(),
                    m_context.swapchain.get_image_format(), (VkSampleCountFlagBits)m_settings.samplesMSAA);

        m_deletionQueue.push_function([=]() { m_gui->cleanup(m_context.device); });
    }
}
VULKAN_ENGINE_NAMESPACE_END