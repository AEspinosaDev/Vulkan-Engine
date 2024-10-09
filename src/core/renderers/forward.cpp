#include <engine/core/renderers/forward.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void ForwardRenderer::on_before_render(Scene *const scene)
{
    Renderer::on_before_render(scene);

    m_renderPipeline.renderpasses[FORWARD]->set_attachment_clear_value(
        {m_settings.clearColor.r,
         m_settings.clearColor.g,
         m_settings.clearColor.b,
         m_settings.clearColor.a});
}

void ForwardRenderer::on_after_render(VkResult &renderResult, Scene *const scene)
{
    Renderer::on_after_render(renderResult, scene);

    if (m_updateShadows)
        update_shadow_quality();
}
void ForwardRenderer::setup_renderpasses()
{
    const uint32_t SHADOW_RES = (uint32_t)m_settings2.shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    // Shadow Pass
    ShadowPass *shadowPass = new ShadowPass(
        &m_context,
        {SHADOW_RES, SHADOW_RES},
        totalImagesInFlight,
        VK_MAX_LIGHTS,
        m_settings.depthFormat);
    m_renderPipeline.push_renderpass(shadowPass);

    // Forward Pass
    ForwardPass *forwardPass = new ForwardPass(
        &m_context,
        m_window->get_extent(),
        totalImagesInFlight,
        m_settings.colorFormat,
        m_settings.depthFormat,
        m_settings.samplesMSAA,
        m_settings2.fxaa ? false : true);
    forwardPass->set_image_dependace_table({{SHADOW, {0}}});
    m_renderPipeline.push_renderpass(forwardPass);

    // FXAA Pass
    FXAAPass *fxaaPass = new FXAAPass(
        &m_context,
        m_window->get_extent(),
        totalImagesInFlight,
        m_settings.colorFormat,
        m_vignette,
        m_settings2.fxaa);
    fxaaPass->set_image_dependace_table({{FORWARD, {0}}});
    m_renderPipeline.push_renderpass(fxaaPass);
    if (!m_settings2.fxaa)
        fxaaPass->set_active(false);
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

    const uint32_t SHADOW_RES = (uint32_t)m_settings2.shadowQuality;
    m_renderPipeline.renderpasses[SHADOW]->set_extent({SHADOW_RES, SHADOW_RES});
    m_renderPipeline.renderpasses[SHADOW]->update();

    m_updateShadows = false;

    connect_renderpass(m_renderPipeline.renderpasses[FORWARD]);
}
VULKAN_ENGINE_NAMESPACE_END