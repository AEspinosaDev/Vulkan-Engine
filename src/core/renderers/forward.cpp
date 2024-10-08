#include <engine/core/renderers/forward.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void ForwardRenderer::on_before_render(Scene *const scene)
{
    Renderer::on_before_render(scene);

    m_renderPipeline.renderpasses[1]->set_attachment_clear_value(
        {m_settings.clearColor.r,
         m_settings.clearColor.g,
         m_settings.clearColor.b,
         m_settings.clearColor.a});
}

void ForwardRenderer::on_after_render(VkResult &renderResult, Scene *const scene)
{
    Renderer::on_after_render(renderResult, scene);

    if (m_settings.updateShadows)
		update_shadow_quality();
}
void ForwardRenderer::setup_renderpasses()
{
    const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    ForwardPass *forwardPass = new ForwardPass(
        &m_context,
        m_window->get_extent(),
        totalImagesInFlight,
        m_settings.colorFormat,
        m_settings.depthFormat,
        m_settings.AAtype);
    forwardPass->set_image_dependace_table({{0, {0}}});

    ShadowPass *shadowPass = new ShadowPass(
        &m_context,
        {SHADOW_RES, SHADOW_RES},
        totalImagesInFlight,
        VK_MAX_LIGHTS,
        m_settings.depthFormat);

    m_renderPipeline.push_renderpass(shadowPass);
    m_renderPipeline.push_renderpass(forwardPass);
}

void ForwardRenderer::init_resources()
{
    Renderer::init_resources();

    m_vignette = Mesh::create_quad();
    upload_geometry_data(m_vignette->get_geometry());
}

void ForwardRenderer::clean_Resources()
{
    Renderer::clean_Resources();

    m_vignette->get_geometry()->get_render_data().vbo.cleanup(m_context.memory);
    m_vignette->get_geometry()->get_render_data().ibo.cleanup(m_context.memory);
}

void ForwardRenderer::update_shadow_quality()
{
    m_context.wait_for_device();

    const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
    m_renderPipeline.renderpasses[SHADOW]->set_extent({SHADOW_RES, SHADOW_RES});
    m_renderPipeline.renderpasses[SHADOW]->update();

    m_settings.updateShadows = false;

    connect_renderpass(m_renderPipeline.renderpasses[FORWARD]);
}
VULKAN_ENGINE_NAMESPACE_END