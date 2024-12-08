#ifndef FORWARD_H
#define FORWARD_H

#include <engine/core/passes/bloom_pass.h>
#include <engine/core/passes/forward_pass.h>
#include <engine/core/passes/postprocess_pass.h>
#include <engine/core/passes/variance_shadow_pass.h>

#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using forward rendering. Fully parametrizable.
*/
class ForwardRenderer : public BaseRenderer
{

    enum RendererPasses
    {
        SHADOW_PASS     = 0,
        FORWARD_PASS    = 1,
        BLOOM_PASS      = 2,
        TONEMAPPIN_PASS = 3,
        FXAA_PASS       = 4,
    };

    ShadowResolution m_shadowQuality = ShadowResolution::MEDIUM;
    bool             m_updateShadows = false;

  public:
    ForwardRenderer(Core::IWindow* window)
        : BaseRenderer(window) {
    }
    ForwardRenderer(Core::IWindow*   window,
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