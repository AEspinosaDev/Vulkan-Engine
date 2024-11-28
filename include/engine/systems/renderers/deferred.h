#ifndef DEFERRED_H
#define DEFERRED_H

#include <engine/core/renderpasses/composition_pass.h>
#include <engine/core/renderpasses/fxaa_pass.h>
#include <engine/core/renderpasses/geometry_pass.h>
#include <engine/core/renderpasses/variance_shadow_pass.h>
#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using deferred rendering. Fully parametrizable.
*/
class DeferredRenderer : public BaseRenderer
{
    enum RenderPasses
    {
        SHADOW_PASS      = 0,
        GEOMETRY_PASS    = 1,
        COMPOSITION_PASS = 2,
        FXAA_PASS        = 3
    };

    bool             m_softwareAA    = true; //FXAA for now
    ShadowResolution m_shadowQuality = ShadowResolution::MEDIUM;

    //Query
    bool             m_updateShadows = false;

  public:
    DeferredRenderer(Core::IWindow* window)
        : BaseRenderer(window) {
    }
    DeferredRenderer(Core::IWindow*   window,
                     bool             softwareAA    = true,
                     ShadowResolution shadowQuality = ShadowResolution::MEDIUM,
                     RendererSettings settings      = {})
        : BaseRenderer(window, settings)
        , m_softwareAA(softwareAA)
        , m_shadowQuality(shadowQuality) {
    }

    inline void set_shadow_quality(ShadowResolution quality) {
        m_shadowQuality = quality;
        if (m_initialized)
            m_updateShadows = true;
    }

  protected:
    virtual void on_before_render(Core::Scene* const scene);

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene);

    virtual void create_renderpasses();
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif