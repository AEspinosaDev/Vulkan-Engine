#ifndef DEFERRED_H
#define DEFERRED_H

#include <engine/core/passes/bloom_pass.h>
#include <engine/core/passes/composition_pass.h>
#include <engine/core/passes/geometry_pass.h>
#include <engine/core/passes/postprocess_pass.h>
#include <engine/core/passes/precomposition_pass.h>
#include <engine/core/passes/variance_shadow_pass.h>

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
        SHADOW_PASS         = 0,
        GEOMETRY_PASS       = 1,
        PRECOMPOSITION_PASS = 2,
        COMPOSITION_PASS    = 3,
        BLOOM_PASS          = 4,
        TONEMAPPIN_PASS     = 5,
        FXAA_PASS           = 6,
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
    inline void set_SSR_settings(Core::SSRSettings settings) {
        static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])->set_SSR_settings(settings);
    };
    inline Core::SSRSettings get_SSR_settings() const {
        return static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])->get_SSR_settings();
    };
    inline void set_SSAO_settings(Core::SSAOSettings settings) {
        static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])->enable_AO(settings.enabled);
        static_cast<Core::PreCompositionPass*>(m_passes[PRECOMPOSITION_PASS])->set_SSAO_settings(settings);
    };
    inline Core::SSAOSettings get_SSAO_settings() const {
        return static_cast<Core::PreCompositionPass*>(m_passes[PRECOMPOSITION_PASS])->get_SSAO_settings();
    };
    inline void set_shading_output(Core::OutputBuffer output) {
        static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])->set_output_buffer(output);
    }
    inline Core::OutputBuffer get_shading_output() const {
        return static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])->get_output_buffer();
    }
    inline void set_clearcolor(Vec4 c) {
        BaseRenderer::set_clearcolor(c);
        m_passes[GEOMETRY_PASS]->set_attachment_clear_value(
            {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 2);
    }

  protected:
    virtual void on_before_render(Core::Scene* const scene);

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene);

    virtual void create_renderpasses();
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif