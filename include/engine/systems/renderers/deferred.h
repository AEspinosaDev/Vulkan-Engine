#ifndef DEFERRED_H
#define DEFERRED_H

#include <engine/systems/renderers/renderer.h>

#include <engine/core/passes/bloom_pass.h>
#include <engine/core/passes/composition_pass.h>
#include <engine/core/passes/enviroment_pass.h>
#include <engine/core/passes/geometry_pass.h>
#include <engine/core/passes/postprocess_pass.h>
#include <engine/core/passes/precomposition_pass.h>
#include <engine/core/passes/sky_pass.h>
#include <engine/core/passes/tonemapping_pass.h>
#include <engine/core/passes/variance_shadow_pass.h>
#include <engine/core/passes/voxelization_pass.h>
#include <engine/core/passes/gui_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using deferred rendering. Fully parametrizable.
*/
class DeferredRenderer : public BaseRenderer
{
  protected:
    enum RendererPasses
    {
        SKY_PASS            = 0,
        ENVIROMENT_PASS     = 1,
        SHADOW_PASS         = 2,
        VOXELIZATION_PASS   = 3,
        GEOMETRY_PASS       = 4,
        PRECOMPOSITION_PASS = 5,
        COMPOSITION_PASS    = 6,
        BLOOM_PASS          = 7,
        TONEMAPPIN_PASS     = 8,
        FXAA_PASS           = 9,
        GUI_PASS            = 10,
    };

    // Graphic Settings
    ShadowResolution m_shadowQuality = ShadowResolution::MEDIUM;

    // Query
    bool m_updateShadows = false;
    bool m_updateGI      = false;

    virtual void on_before_render(Core::Scene* const scene);

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene);

    virtual void create_passes();

    virtual void update_enviroment(Core::Skybox* const skybox);

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

    inline ShadowResolution get_shadow_quality() {
        return m_shadowQuality;
    }
    inline void set_shadow_quality(ShadowResolution quality) {
        m_shadowQuality = quality;
        if (m_initialized)
            m_updateShadows = true;
    }
    inline float get_bloom_strength() {
        return get_pass<Core::BloomPass*>(BLOOM_PASS)->get_bloom_strength();
    }
    inline void set_bloom_strength(float st) {
        get_pass<Core::BloomPass*>(BLOOM_PASS)->set_bloom_strength(st);
    }
    inline void set_SSR_settings(Core::SSR settings) {
        get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->set_SSR_settings(settings);
    };
    inline Core::SSR get_SSR_settings() {
        return get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->get_SSR_settings();
    };
    inline void set_VXGI_settings(Core::VXGI settings) {
        if (get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->get_VXGI_settings().resolution != settings.resolution)
            m_updateGI = true;
        get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->set_VXGI_settings(settings);
        m_passes[VOXELIZATION_PASS]->set_active(settings.enabled);
    };
    inline Core::VXGI get_VXGI_settings() {
        return get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->get_VXGI_settings();
    };
    inline void set_SSAO_settings(Core::AO settings) {
        get_pass<Core::PreCompositionPass*>(PRECOMPOSITION_PASS)->set_SSAO_settings(settings);
        get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->enable_AO(settings.enabled);
        get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->set_AO_type(static_cast<int>(settings.type));
        m_passes[PRECOMPOSITION_PASS]->set_active(settings.enabled && settings.type != Core::AOType::VXAO);
    };
    inline Core::AO get_SSAO_settings() {
        return get_pass<Core::PreCompositionPass*>(PRECOMPOSITION_PASS)->get_SSAO_settings();
    };
    inline void set_shading_output(Core::OutputBuffer output) {
        get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->set_output_buffer(output);
    }
    inline Core::OutputBuffer get_shading_output() {
        return get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->get_output_buffer();
    }
    inline float get_exposure() {
        auto pass = get_pass<Core::TonemappingPass*>(TONEMAPPIN_PASS);
        return pass->get_exposure();
    }
    inline void set_exposure(float exposure) {
        get_pass<Core::TonemappingPass*>(TONEMAPPIN_PASS)->set_exposure(exposure);
    }
    inline Core::TonemappingType get_tonemapping_type() {
        return get_pass<Core::TonemappingPass*>(TONEMAPPIN_PASS)->get_tonemapping_type();
    }
    inline void set_tonemapping_type(Core::TonemappingType type) {
        get_pass<Core::TonemappingPass*>(TONEMAPPIN_PASS)->set_tonemapping_type(type);
    }
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif