#ifndef VK_RENDERER_WIDGET
#define VK_RENDERER_WIDGET

#include <engine/utilities/vk_widgets.h>
#include <engine/vk_renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class RendererSettingsWidget : public Widget
{

protected:
    Renderer *m_renderer;
    virtual void render();

public:
    RendererSettingsWidget(Renderer *r) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_renderer(r) {}

    inline Renderer *get_renderer() const { return m_renderer; }
    inline void set_renderer(Renderer *r) { m_renderer = r; }
};

VULKAN_ENGINE_NAMESPACE_END
#endif
