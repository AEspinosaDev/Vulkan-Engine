#ifndef DEFERRED_H
#define DEFERRED_H

#include <engine/core/passes/bloom_pass.h>
#include <engine/core/passes/composition_pass.h>
#include <engine/core/passes/geometry_pass.h>
#include <engine/core/passes/postprocess_pass.h>
#include <engine/core/passes/precomposition_pass.h>
#include <engine/core/passes/variance_shadow_pass.h>
#include <engine/core/passes/voxelization_pass.h>

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
        VOXELIZATION_PASS   = 1,
        GEOMETRY_PASS       = 2,
        PRECOMPOSITION_PASS = 3,
        COMPOSITION_PASS    = 4,
        BLOOM_PASS          = 5,
        TONEMAPPIN_PASS     = 6,
        FXAA_PASS           = 7,
    };

    // Graphic Settings
    ShadowResolution   m_shadowQuality = ShadowResolution::MEDIUM;
    Core::SSR          m_SSR           = {};
    Core::VXGI         m_VXGI          = {};
    Core::SSAOSettings m_SSAO          = {};
    float              m_bloomStrength = 0.05f;

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
        return m_bloomStrength;
    }
    inline void set_bloom_strength(float st) {
        m_bloomStrength = st;
    }
    inline void set_SSR_settings(Core::SSR settings) {
        m_SSR = settings;
    };
    inline Core::SSR get_SSR_settings() const {
        return m_SSR;
    };
    inline void set_VXGI_settings(Core::VXGI settings) {
        m_VXGI = settings;
    };
    inline Core::VXGI get_VXGI_settings() const {
        return m_VXGI;
    };
    inline void set_SSAO_settings(Core::SSAOSettings settings) {
        m_SSAO = settings;
    };
    inline Core::SSAOSettings get_SSAO_settings() const {
        return m_SSAO;
    };
    inline void set_shading_output(Core::OutputBuffer output) {
        static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])->set_output_buffer(output);
    }
    inline Core::OutputBuffer get_shading_output() const {
        return static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])->get_output_buffer();
    }
    inline void set_clearcolor(Vec4 c) {
        BaseRenderer::set_clearcolor(c);
       
    }

  protected:
    virtual void on_before_render(Core::Scene* const scene);

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene);

    virtual void create_passes();
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif