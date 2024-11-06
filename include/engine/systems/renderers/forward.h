#ifndef FORWARD_H
#define FORWARD_H

#include <engine/core/renderpasses/forward_pass.h>
#include <engine/core/renderpasses/fxaa_pass.h>
#include <engine/core/renderpasses/panorama_conversion_pass.h>
#include <engine/core/renderpasses/shadow_pass.h>
#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

struct ForwardRendererSettings {

    ShadowResolution shadowQuality{ShadowResolution::MEDIUM};
    bool             fxaa{false};
};

/*
Renders a given scene data to a given window using forward rendering. Fully parametrizable.
*/
class ForwardRenderer : public BaseRenderer
{
    ForwardRendererSettings m_settings2{};

    enum RenderPasses
    {
        SHADOW  = 0,
        FORWARD = 1,
        FXAA    = 2
    };

    bool m_updateShadows{false};

  public:
    ForwardRenderer(Core::IWindow* window)
        : BaseRenderer(window) {
    }
    ForwardRenderer(Core::IWindow* window, RendererSettings settings, ForwardRendererSettings settings2)
        : BaseRenderer(window, settings)
        , m_settings2(settings2) {
    }

    inline void set_shadow_quality(ShadowResolution quality) {
        m_settings2.shadowQuality = quality;
        if (m_initialized)
            m_updateShadows = true;
    }

  protected:
    virtual void on_before_render(Core::Scene* const scene);

    virtual void on_after_render(VkResult& renderResult, Core::Scene* const scene);

    virtual void setup_renderpasses();

    virtual void update_shadow_quality();
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif