#include "engine/utilities/vk_renderer_widget.h"

namespace vke
{
    void RendererSettingsWidget::render()
    {

        ImGui::SeparatorText("Global settings");
        glm::vec3 clearColor = m_renderer->get_settings().clearColor;
        if (ImGui::ColorEdit3("Clear Color", (float *)&clearColor))
        {
            m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
        }
        bool depthTest = m_renderer->get_settings().depthTest;
        bool depthWrite = m_renderer->get_settings().depthWrite;
        if (ImGui::Checkbox("Depth Test", &depthTest))
        {
            m_renderer->enable_depth_test(depthTest);
        }
        if (ImGui::Checkbox("Depth Write", &depthWrite))
        {
            m_renderer->enable_depth_writes(depthWrite);
        }
        ImGui::BulletText("Gamma Correction Enabled");
        ImGui::BulletText("Device Dependable Anisotropic Filter Enabled");

        const char *items[] = {"NONE", "MSAAx4", "MSAAx8"};
        static int item_current = 2;
        if (ImGui::Combo("Antialiasing", &item_current, items, IM_ARRAYSIZE(items)))
        {
            switch (item_current)
            {
            case 0:
                m_renderer->set_antialiasing(_NONE);
                break;
            case 1:
                m_renderer->set_antialiasing(MSAA_x4);
                break;
            case 2:
                m_renderer->set_antialiasing(MSAA_x8);
                break;
            }
        }

        const char *res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

        static int res_current = 1;
        if (ImGui::Combo("Shadows Quality", &res_current, res, IM_ARRAYSIZE(res)))
        {
            // switch (res_current)
            // {
            // case 0:
            //     m_renderer->m_settings.shadowResolution = ShadowResolution::VERY_LOW;
            //     break;
            // case 1:
            //     m_renderer->m_settings.shadowResolution = ShadowResolution::LOW;
            //     break;
            // case 2:
            //     m_renderer->m_settings.shadowResolution = ShadowResolution::MEDIUM;
            //     break;
            // case 3:
            //     m_renderer->m_settings.shadowResolution = ShadowResolution::HIGH;
            //     break;
            // case 4:
            //     m_renderer->m_settings.shadowResolution = ShadowResolution::ULTRA;
            //     break;
            // }
        }
    }
}