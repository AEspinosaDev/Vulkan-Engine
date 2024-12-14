#include <engine/tools/renderer_widget.h>
VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Tools {
void RendererSettingsWidget::render() {

    ImGui::SeparatorText("Renderer Settings");

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Screen Sync", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        switch (sync_current)
        {
        case 0:
            m_renderer->set_sync_type(SyncType::NONE);
            break;
        case 1:
            m_renderer->set_sync_type(SyncType::MAILBOX);
            break;
        case 2:
            m_renderer->set_sync_type(SyncType::VERTICAL);
            break;
        }
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
    }
}
} // namespace Tools
void Tools::ForwardRendererWidget::render() {
    ImGui::SeparatorText("Renderer Settings");

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Screen Sync", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        switch (sync_current)
        {
        case 0:
            m_renderer->set_sync_type(SyncType::NONE);
            break;
        case 1:
            m_renderer->set_sync_type(SyncType::MAILBOX);
            break;
        case 2:
            m_renderer->set_sync_type(SyncType::VERTICAL);
            break;
        }
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
    }

    const char* res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

    ShadowResolution res_currentr = m_renderer->get_shadow_quality();
    int              res_current;
    switch (res_currentr)
    {
    case ShadowResolution::VERY_LOW:
        res_current = 0;
        break;
    case ShadowResolution::LOW:
        res_current = 1;
        break;
    case ShadowResolution::MEDIUM:
        res_current = 2;
        break;
    case ShadowResolution::HIGH:
        res_current = 3;
        break;
    case ShadowResolution::ULTRA:
        res_current = 4;
        break;
    }

    if (ImGui::Combo("Shadow Quality", &res_current, res, IM_ARRAYSIZE(res)))
    {
        switch (res_current)
        {
        case 0:
            m_renderer->set_shadow_quality(ShadowResolution::VERY_LOW);
            break;
        case 1:
            m_renderer->set_shadow_quality(ShadowResolution::LOW);
            break;
        case 2:
            m_renderer->set_shadow_quality(ShadowResolution::MEDIUM);
            break;
        case 3:
            m_renderer->set_shadow_quality(ShadowResolution::HIGH);
            break;
        case 4:
            m_renderer->set_shadow_quality(ShadowResolution::ULTRA);
            break;
        }
    }
    ImGui::Separator();

    float bloomIntensity = m_renderer->get_bloom_strength();
    if (ImGui::DragFloat("Bloom Intensity", &bloomIntensity, 0.01f, 0.0f, 0.5f))
    {
        m_renderer->set_bloom_strength(bloomIntensity);
    }
}
// namespace Tools
void Tools::DeferredRendererWidget::render() {
    ImGui::SeparatorText("Renderer Settings");

    const char* outputTypes[] = {"LIGHTING", "ALBEDO", "NORMALS", "POSITION", "MATERIAL","SSR","AO"};
    static int  otype_current = static_cast<int>(m_renderer->get_shading_output());
    if (ImGui::Combo("Shading Output", &otype_current, outputTypes, IM_ARRAYSIZE(outputTypes)))
    {
        switch (otype_current)
        {
        case 0:
            m_renderer->set_shading_output(Core::OutputBuffer::LIGHTING);
            break;
        case 1:
            m_renderer->set_shading_output(Core::OutputBuffer::ALBEDO);
            break;
        case 2:
            m_renderer->set_shading_output(Core::OutputBuffer::NORMAL);
            break;
        case 3:
            m_renderer->set_shading_output(Core::OutputBuffer::POSITION);
            break;
        case 4:
            m_renderer->set_shading_output(Core::OutputBuffer::MATERIAL);
            break;
        case 5:
            m_renderer->set_shading_output(Core::OutputBuffer::LIGHTING);
            break;
        }
    }

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Screen Sync", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        switch (sync_current)
        {
        case 0:
            m_renderer->set_sync_type(SyncType::NONE);
            break;
        case 1:
            m_renderer->set_sync_type(SyncType::MAILBOX);
            break;
        case 2:
            m_renderer->set_sync_type(SyncType::VERTICAL);
            break;
        }
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
    }

    const char* res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

    ShadowResolution res_currentr = m_renderer->get_shadow_quality();
    int              res_current;
    switch (res_currentr)
    {
    case ShadowResolution::VERY_LOW:
        res_current = 0;
        break;
    case ShadowResolution::LOW:
        res_current = 1;
        break;
    case ShadowResolution::MEDIUM:
        res_current = 2;
        break;
    case ShadowResolution::HIGH:
        res_current = 3;
        break;
    case ShadowResolution::ULTRA:
        res_current = 4;
        break;
    }

    if (ImGui::Combo("Shadow Quality", &res_current, res, IM_ARRAYSIZE(res)))
    {
        switch (res_current)
        {
        case 0:
            m_renderer->set_shadow_quality(ShadowResolution::VERY_LOW);
            break;
        case 1:
            m_renderer->set_shadow_quality(ShadowResolution::LOW);
            break;
        case 2:
            m_renderer->set_shadow_quality(ShadowResolution::MEDIUM);
            break;
        case 3:
            m_renderer->set_shadow_quality(ShadowResolution::HIGH);
            break;
        case 4:
            m_renderer->set_shadow_quality(ShadowResolution::ULTRA);
            break;
        }
    }
    ImGui::Separator();

    float bloomIntensity = m_renderer->get_bloom_strength();

    if (ImGui::DragFloat("Bloom Intensity", &bloomIntensity, 0.01f, 0.0f, 0.5f))
    {
        m_renderer->set_bloom_strength(bloomIntensity);
    }
    ImGui::Separator();
    Core::SSRSettings settings_SSR = m_renderer->get_SSR_settings();
    bool              ssrEnabled   = (bool)settings_SSR.enabled;
    if (ImGui::Checkbox("Enable SSR", &ssrEnabled))
    {
        settings_SSR.enabled = ssrEnabled;
        m_renderer->set_SSR_settings(settings_SSR);
    }
    if (settings_SSR.enabled)
    {
        int ssrMaxSteps    = (int)settings_SSR.maxSteps;
        int ssrRefineSteps = (int)settings_SSR.binaryRefinementSteps;
        if (ImGui::DragInt("SSR Steps", &ssrMaxSteps, 1, 1, 254))
        {
            settings_SSR.maxSteps = ssrMaxSteps;
            m_renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragFloat("SSR Stride", &settings_SSR.stride, 0.01f, 0.0f, 10.0f))
        {
            m_renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragFloat("SSR Thickness", &settings_SSR.thickness, 0.01f, 0.0f, 5.0f))
        {
            m_renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragInt("SSR Refinement Steps", &ssrRefineSteps, 1, 1, 10))
        {
            settings_SSR.binaryRefinementSteps = ssrRefineSteps;
            m_renderer->set_SSR_settings(settings_SSR);
        }
    }
}
// namespace Tools
VULKAN_ENGINE_NAMESPACE_END