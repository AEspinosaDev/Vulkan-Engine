#include <engine/utilities/vk_renderer_widget.h>
VULKAN_ENGINE_NAMESPACE_BEGIN
void RendererSettingsWidget::render()
{

    ImGui::SeparatorText("Global settings");
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
    bool hwBias =  m_renderer->get_settings().enableHardwareDepthBias;
    if (ImGui::Checkbox("Hardware Depth Bias", &hwBias))
    {
        m_renderer->set_hardware_depth_bias(hwBias);
    };

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
VULKAN_ENGINE_NAMESPACE_END