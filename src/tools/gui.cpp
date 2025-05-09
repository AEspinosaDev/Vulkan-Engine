#include <engine/tools/gui.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Tools {
void GUIOverlay::render() {
    PROFILING_EVENT()

    if (ImGui::GetCurrentContext())
    {

        switch (m_colorProfile)
        {
        case BRIGHT:
            ImGui::StyleColorsLight();
            break;
        case DARK:
            ImGui::StyleColorsDark();
            break;
        case CLASSIC:
            ImGui::StyleColorsClassic();
            break;
        default:
            break;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for (auto p : m_panels)
        {
            p->render(m_extent);
            p->set_is_resized(m_resized);
        }
        ImGui::Render();
    }

    m_resized = false;
}

} // namespace Tools
VULKAN_ENGINE_NAMESPACE_END