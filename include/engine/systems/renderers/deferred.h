#ifndef DEFERRED_H
#define DEFERRED_H

#include <engine/core/passes/variance_shadow_pass.h>
#include <engine/core/passes/composition_pass.h>
#include <engine/core/passes/postprocess_pass.h>
#include <engine/core/passes/geometry_pass.h>
#include <engine/core/passes/bloom_pass.h>
#include <engine/core/passes/SSR_pass.h>

#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using deferred rendering. Fully parametrizable.
*/
class DeferredRenderer : public BaseRenderer
{
    enum RendererPasses
    {
        SHADOW_PASS      = 0,
        GEOMETRY_PASS    = 1,
        COMPOSITION_PASS = 2,
        SSR_PASS         = 3,
        BLOOM_PASS       = 4,
        TONEMAPPIN_PASS  = 5,
        FXAA_PASS        = 6,
    };

    ShadowResolution m_shadowQuality = ShadowResolution::MEDIUM;

    // Query
    bool m_updateShadows = false;

  public:
    DeferredRenderer(Core::IWindow* window)
        : BaseRenderer(window) {
    }
    DeferredRenderer(Core::IWindow*   window,
                     ShadowResolution shadowQuality = ShadowResolution::MEDIUM,
                     RendererSettings settings      = {})
        : BaseRenderer(window, settings)
        , m_shadowQuality(shadowQuality) {
    }

    inline ShadowResolution get_shadow_quality() const {
        return m_shadowQuality;
    }
    inline void set_shadow_quality(ShadowResolution quality) {
        m_shadowQuality = quality;
        if (m_initialized)
            m_updateShadows = true;
    }
    inline float get_bloom_strength() const {
        if (m_passes[BLOOM_PASS])
        {
            return static_cast<Core::BloomPass*>(m_passes[BLOOM_PASS])->get_bloom_strength();
        }
        return 0.0f;
    }
    inline void set_bloom_strength(float st) {
        if (m_passes[BLOOM_PASS])
        {
            static_cast<Core::BloomPass*>(m_passes[BLOOM_PASS])->set_bloom_strength(st);
        }
    }

  protected:
    virtual void on_before_render(Core::Scene* const scene);

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene);

    virtual void create_renderpasses();
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif