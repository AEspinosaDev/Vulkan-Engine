#ifndef DEFERRED_H
#define DEFERRED_H

#include <engine/systems/renderers/renderer.h>

#include <engine/render/passes/TAA_pass.h>
#include <engine/render/passes/bloom_pass.h>
#include <engine/render/passes/composition_pass.h>
#include <engine/render/passes/enviroment_pass.h>
#include <engine/render/passes/geometry_pass.h>
#include <engine/render/passes/gui_pass.h>
#include <engine/render/passes/postprocess_pass.h>
#include <engine/render/passes/precomposition_pass.h>
#include <engine/render/passes/sky_pass.h>
#include <engine/render/passes/tonemapping_pass.h>
#include <engine/render/passes/variance_shadow_pass.h>
#include <engine/render/passes/voxelization_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using deferred rendering. Fully parametrizable.
*/
class DeferredRenderer : public BaseRenderer
{
  protected:
    // Query
    bool m_updateShadows = false;
    bool m_updateGI      = false;

    virtual void on_before_render(Core::Scene* const scene) override;

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene) override;

    virtual void create_passes() override;

    virtual void update_enviroment(Core::Skybox* const skybox);

  public:
    enum Passes
    {
        SKY_PASS            = 0,
        ENVIROMENT_PASS     = 1,
        SHADOW_PASS         = 2,
        VOXELIZATION_PASS   = 3,
        GEOMETRY_PASS       = 4,
        PRECOMPOSITION_PASS = 5,
        COMPOSITION_PASS    = 6,
        BLOOM_PASS          = 7,
        AA_PASS             = 8,
        TONEMAPPIN_PASS     = 9,
        GUI_PASS            = 10, /* UNUSED IF HEADLESS */
    };

    DeferredRenderer(const ptr<Core::IWindow>& window)
        : BaseRenderer(window) {
    }
    DeferredRenderer(const ptr<Core::IWindow>& window, RendererSettings settings = {})
        : BaseRenderer(window, settings) {
    }
    // Headless instantiation
    DeferredRenderer(Extent2D displayExtent = {800, 800})
        : BaseRenderer(displayExtent) {
    }

    virtual inline void set_settings(RendererSettings settings) override {
        if (m_settings.shadowQuality != settings.shadowQuality)
            m_updateShadows = true;

        BaseRenderer::set_settings(settings);
    }
    inline float get_bloom_strength() {
        return get_pass<Render::BloomPass>(BLOOM_PASS)->get_bloom_strength();
    }
    inline void set_bloom_strength(float st) {
        get_pass<Render::BloomPass>(BLOOM_PASS)->set_bloom_strength(st);
    }
    inline void set_SSR_settings(Render::SSR settings) {
        get_pass<Render::CompositionPass>(COMPOSITION_PASS)->set_SSR_settings(settings);
    };
    inline Render::SSR get_SSR_settings() {
        return get_pass<Render::CompositionPass>(COMPOSITION_PASS)->get_SSR_settings();
    };
    inline void set_VXGI_settings(Render::VXGI settings) {
        if (get_pass<Render::CompositionPass>(COMPOSITION_PASS)->get_VXGI_settings().resolution != settings.resolution)
            m_updateGI = true;
        get_pass<Render::CompositionPass>(COMPOSITION_PASS)->set_VXGI_settings(settings);
        m_passes[VOXELIZATION_PASS]->set_active(settings.enabled);
    };
    inline Render::VXGI get_VXGI_settings() {
        return get_pass<Render::CompositionPass>(COMPOSITION_PASS)->get_VXGI_settings();
    };
    inline void set_SSAO_settings(Render::AO settings) {
        get_pass<Render::PreCompositionPass>(PRECOMPOSITION_PASS)->set_SSAO_settings(settings);
        get_pass<Render::CompositionPass>(COMPOSITION_PASS)->enable_AO(settings.enabled);
        get_pass<Render::CompositionPass>(COMPOSITION_PASS)->set_AO_type(static_cast<int>(settings.type));
        m_passes[PRECOMPOSITION_PASS]->set_active(settings.enabled && settings.type != Render::AOType::VXAO);
    };
    inline Render::AO get_SSAO_settings() {
        return get_pass<Render::PreCompositionPass>(PRECOMPOSITION_PASS)->get_SSAO_settings();
    };
    inline void set_shading_output(Render::OutputBuffer output) {
        get_pass<Render::CompositionPass>(COMPOSITION_PASS)->set_output_buffer(output);
    }
    inline Render::OutputBuffer get_shading_output() {
        return get_pass<Render::CompositionPass>(COMPOSITION_PASS)->get_output_buffer();
    }
    inline float get_exposure() {
        auto pass = get_pass<Render::TonemappingPass>(TONEMAPPIN_PASS);
        return pass->get_exposure();
    }
    inline void set_exposure(float exposure) {
        get_pass<Render::TonemappingPass>(TONEMAPPIN_PASS)->set_exposure(exposure);
    }
    inline Render::TonemappingType get_tonemapping_type() {
        return get_pass<Render::TonemappingPass>(TONEMAPPIN_PASS)->get_tonemapping_type();
    }
    inline void set_tonemapping_type(Render::TonemappingType type) {
        get_pass<Render::TonemappingPass>(TONEMAPPIN_PASS)->set_tonemapping_type(type);
    }
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif