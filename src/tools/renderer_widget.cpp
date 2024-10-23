#include <engine/tools/renderer_widget.h>
VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Tools
{
void RendererSettingsWidget::render()
{

    ImGui::SeparatorText("Global settings");

    const char *renderTypes[] = {"FORWARD", "DEFERRED"};
    // static int type_current = static_cast<int>(m_renderer->get_settings().renderingType);
    // if (ImGui::Combo("Rendering Method", &type_current, renderTypes, IM_ARRAYSIZE(renderTypes)))
    // {
    // 	switch (type_current)
    // 	{
    // 	case 0:
    // 		m_renderer->set_rendering_method(RendererType::TFORWARD);
    // 		break;
    // 	case 1:
    // 		m_renderer->set_rendering_method(RendererType::TDEFERRED);
    // 		break;
    // 	}
    // }

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

    // const char *items[] = {"NONE", "MSAAx4", "MSAAx8", "FXAA"};
    // MSAASamples item_currentr = m_renderer->get_settings().AAtype;
    // int item_current;
    // switch (item_currentr)
    // {
    // case MSAASamples::_NONE:
    // 	item_current = 0;
    // 	break;
    // case MSAASamples::MSAA_x4:
    // 	item_current = 1;
    // 	break;
    // case MSAASamples::MSAA_x8:
    // 	item_current = 2;
    // 	break;
    // case MSAASamples::FXAA:
    // 	item_current = 3;
    // 	break;
    // }
    // if (ImGui::Combo("Antialiasing", &item_current, items, IM_ARRAYSIZE(items)))
    // {
    // 	/*switch (item_current)
    // 	{
    // 	case 0:
    // 		m_renderer->set_antialiasing(FXAA);
    // 		break;
    // 	case 1:
    // 		m_renderer->set_antialiasing(_NONE);
    // 		break;
    // 	case 4:
    // 		m_renderer->set_antialiasing(MSAA_x4);
    // 		break;
    // 	case 8:
    // 		m_renderer->set_antialiasing(MSAA_x8);
    // 		break;
    // 	}*/
    // }

    const char *syncs[] = {"NONE", "MAILBOX", "VSYNC"};
    static int sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Screen Sync", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        switch (sync_current)
        {
        case 0:
            m_renderer->set_sync_type(SyncType::NONE);
            break;
        case 1:
            m_renderer->set_sync_type(SyncType::MAILBOX_SYNC);
            break;
        case 2:
            m_renderer->set_sync_type(SyncType::V_SYNC);
            break;
        }
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float *)&clearColor))
    {
        m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
    }

    const char *ssaoType[] = {"SSAO", "Unsharp SSAO"};
    // static int currentSSAO = static_cast<int>(m_renderer->get_ssao_type());
    // if (ImGui::Combo("SSAO Implementation", &currentSSAO, ssaoType, IM_ARRAYSIZE(ssaoType)))
    // {
    // 	switch (currentSSAO)
    // 	{
    // 	case 0:
    // 		m_renderer->set_ssao_type(AmbientOcclusionType::SSAO);
    // 		break;
    // 	case 1:
    // 		m_renderer->set_ssao_type(AmbientOcclusionType::USSAO);
    // 		break;
    // 	}
    // };

    ImGui::BulletText("Gamma Correction Enabled");
    ImGui::BulletText("Device Dependable Anisotropic Filter Enabled");

    const char *res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

    // ShadowResolution res_currentr = m_renderer->get_settings().shadowResolution;
    // int res_current;
    // switch (res_currentr)
    // {
    // case ShadowResolution::VERY_LOW:
    // 	res_current = 0;
    // 	break;
    // case ShadowResolution::LOW:
    // 	res_current = 1;
    // 	break;
    // case ShadowResolution::MEDIUM:
    // 	res_current = 2;
    // 	break;
    // case ShadowResolution::HIGH:
    // 	res_current = 3;
    // 	break;
    // case ShadowResolution::ULTRA:
    // 	res_current = 4;
    // 	break;
    // }

    // if (ImGui::Combo("Shadows Quality", &res_current, res, IM_ARRAYSIZE(res)))
    // {
    // 	switch (res_current)
    // 	{
    // 	case 0:
    // 		m_renderer->set_shadow_quality(ShadowResolution::VERY_LOW);
    // 		break;
    // 	case 1:
    // 		m_renderer->set_shadow_quality(ShadowResolution::LOW);
    // 		break;
    // 	case 2:
    // 		m_renderer->set_shadow_quality(ShadowResolution::MEDIUM);
    // 		break;
    // 	case 3:
    // 		m_renderer->set_shadow_quality(ShadowResolution::HIGH);
    // 		break;
    // 	case 4:
    // 		m_renderer->set_shadow_quality(ShadowResolution::ULTRA);
    // 		break;
    // 	}
    // }
}
} // namespace Tools
VULKAN_ENGINE_NAMESPACE_END