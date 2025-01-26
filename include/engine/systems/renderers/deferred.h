#ifndef DEFERRED_H
#define DEFERRED_H

#include <engine/systems/renderers/renderer.h>

#include <engine/core/passes/bloom_pass.h>
#include <engine/core/passes/composition_pass.h>
#include <engine/core/passes/enviroment_pass.h>
#include <engine/core/passes/geometry_pass.h>
#include <engine/core/passes/postprocess_pass.h>
#include <engine/core/passes/precomposition_pass.h>
#include <engine/core/passes/tonemapping_pass.h>
#include <engine/core/passes/variance_shadow_pass.h>
#include <engine/core/passes/voxelization_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using deferred rendering. Fully parametrizable.
*/
class DeferredRenderer : public BaseRenderer
{
    enum RendererPasses
    {
        ENVIROMENT_PASS     = 0,
        SHADOW_PASS         = 1,
        VOXELIZATION_PASS   = 2,
        GEOMETRY_PASS       = 3,
        PRECOMPOSITION_PASS = 4,
        COMPOSITION_PASS    = 5,
        BLOOM_PASS          = 6,
        TONEMAPPIN_PASS     = 7,
        FXAA_PASS           = 8,
    };

    // Graphic Settings
    ShadowResolution m_shadowQuality = ShadowResolution::MEDIUM;
    Core::SSR        m_SSR           = {};
    Core::VXGI       m_VXGI          = {};
    Core::AO         m_AO            = {};
    float            m_bloomStrength = 0.05f;

    // Query
    bool m_updateShadows = false;
    bool m_updateGI      = false;

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
        if (m_VXGI.resolution != settings.resolution)
            m_updateGI = true;
        m_VXGI = settings;
    };
    inline Core::VXGI get_VXGI_settings() const {
        return m_VXGI;
    };
    inline void set_SSAO_settings(Core::AO settings) {
        m_AO = settings;
    };
    inline Core::AO get_SSAO_settings() const {
        return m_AO;
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

  protected:
    virtual void on_before_render(Core::Scene* const scene);

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene);

    virtual void create_passes();

    virtual void update_enviroment(Core::Skybox* const skybox);
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif