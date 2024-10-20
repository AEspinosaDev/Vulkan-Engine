/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef REND_WIDGET_H
#define REND_WIDGET_H

#include <engine/tools/widgets.h>
#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class RendererSettingsWidget : public Widget
{

protected:
    Systems::Renderer *m_renderer;
    virtual void render();

public:
    RendererSettingsWidget( Systems::Renderer *r) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_renderer(r) {}

    inline  Systems::Renderer *get_renderer() const { return m_renderer; }
    inline void set_renderer( Systems::Renderer *r) { m_renderer = r; }
};

VULKAN_ENGINE_NAMESPACE_END
#endif
