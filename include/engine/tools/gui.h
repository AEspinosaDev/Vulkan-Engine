/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GUI_H
#define GUI_H

#include <engine/graphics/device.h>
#include <engine/tools/widgets.h>

// WIP..
// MUCH TO DO HERE, JUST READY FOR A SIMPLE DEMO
VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Tools
{

class GUIOverlay
{
  private:
    ImVec2 m_extent;

    std::vector<Panel *> m_panels;

    GuiColorProfileType m_colorProfile;

    bool m_resized{false};

  public:
    GUIOverlay(float extentX, float extentY, GuiColorProfileType color = GuiColorProfileType::DARK)
        : m_extent({extentX, extentY}), m_colorProfile(color)
    {
    }
    ~GUIOverlay()
    {
        for (auto p : m_panels)
        {
            delete p;
        }
    }
    void render();
    inline void add_panel(Panel *p)
    {
        p->m_parentOverlay = this;
        m_panels.push_back(p);
    }
    inline bool wants_to_handle_input() const
    {
        if (ImGui::GetCurrentContext())
        {
            ImGuiIO &io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard)
                return true;
            else
                return false;
        }
        return false;
    }

    inline math::vec2 get_extent() const
    {
        return {m_extent.x, m_extent.y};
    }
    inline void set_extent(math::vec2 p)
    {
        m_extent = {p.x, p.y};
        m_resized = true;
    }
};
} // namespace Tools

VULKAN_ENGINE_NAMESPACE_END

#endif