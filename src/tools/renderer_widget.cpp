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
void Tools::DeferredRendererWidget::render() {
    ImGui::SeparatorText("Renderer Settings");

    // if (type_current == 1)
    // {
    // 	const char *outputTypes[] = {"LIGHTING", "POSITION", "NORMALS", "ALBEDO", "MATERIAL", "AO"};
    // static int otype_current = static_cast<int>(m_renderer->get_deferred_output_type());
    // if (ImGui::Combo("Shading Output", &otype_current, outputTypes, IM_ARRAYSIZE(outputTypes)))
    // {
    // 	switch (otype_current)
    // 	{
    // 	case 0:
    // 		m_renderer->set_deferred_output_type(0);
    // 		break;
    // 	case 1:
    // 		m_renderer->set_deferred_output_type(1);
    // 		break;
    // 	case 2:
    // 		m_renderer->set_deferred_output_type(2);
    // 		break;
    // 	case 3:
    // 		m_renderer->set_deferred_output_type(3);
    // 		break;
    // 	case 4:
    // 		m_renderer->set_deferred_output_type(4);
    // 		break;
    // 	case 5:
    // 		m_renderer->set_deferred_output_type(5);
    // 		break;
    // 	}
    // }
    // }

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
VULKAN_ENGINE_NAMESPACE_END