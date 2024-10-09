#ifndef FORWARD_H
#define FORWARD_H

#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct ForwardRendererSettings
{

    ShadowResolution shadowQuality{ShadowResolution::MEDIUM};
    bool fxaa{false};
};

/*
Renders a given scene data to a given window using forward rendering. Fully parametrizable.
*/
class ForwardRenderer : public Renderer
{
    ForwardRendererSettings m_settings2{};

    Mesh *m_vignette;

    enum RenderPasses
    {
        SHADOW = 0,
        FORWARD = 1,
        FXAA = 2
    };

    bool m_updateShadows{false};

public:
    ForwardRenderer(Window *window) : Renderer(window) {}
    ForwardRenderer(Window *window, RendererSettings settings, ForwardRendererSettings settings2) : Renderer(window, settings), m_settings2(settings2) {}

    inline void set_shadow_quality(ShadowResolution quality)
    {
        m_settings2.shadowQuality = quality;
        if (m_initialized)
            m_updateShadows = true;
    }

protected:
    virtual void on_before_render(Scene *const scene);

    virtual void on_after_render(VkResult &renderResult, Scene *const scene);

    virtual void setup_renderpasses();

    virtual void init_resources();

    virtual void clean_Resources();

    virtual void update_shadow_quality();
};

VULKAN_ENGINE_NAMESPACE_END

#endif