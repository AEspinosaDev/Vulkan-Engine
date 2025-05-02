#ifndef FORWARD_H
#define FORWARD_H

#include <engine/core/passes/bloom_pass.h>
#include <engine/core/passes/enviroment_pass.h>
#include <engine/core/passes/forward_pass.h>
#include <engine/core/passes/gui_pass.h>
#include <engine/core/passes/postprocess_pass.h>
#include <engine/core/passes/sky_pass.h>
#include <engine/core/passes/tonemapping_pass.h>
#include <engine/core/passes/variance_shadow_pass.h>

#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using forward rendering. Fully parametrizable.
*/
class ForwardRenderer : public BaseRenderer
{
  protected:
    /*Query*/
    bool m_updateShadows = false;

    virtual void on_before_render(Core::Scene* const scene) override;

    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene) override;

    virtual void create_passes() override;

    virtual void update_enviroment(Core::Skybox* const skybox);

  public:
    enum Passes
    {
        SKY_PASS        = 0,
        ENVIROMENT_PASS = 1,
        SHADOW_PASS     = 2,
        FORWARD_PASS    = 3,
        BLOOM_PASS      = 4,
        FXAA_PASS       = 5,
        TONEMAPPIN_PASS = 6,
        GUI_PASS        = 7
    };

    ForwardRenderer(const ptr<Core::IWindow>& window)
        : BaseRenderer(window) {
    }
    ForwardRenderer(const ptr<Core::IWindow>& window, RendererSettings settings = {})
        : BaseRenderer(window, settings) {
    }
    // Headless instantiation
    ForwardRenderer(Extent2D displayExtent = {800, 800})
        : BaseRenderer(displayExtent) {
    }

    virtual inline void set_settings(RendererSettings settings) override {
        if (m_settings.shadowQuality != settings.shadowQuality)
            m_updateShadows = true;

        BaseRenderer::set_settings(settings);
    }

    inline float get_bloom_strength() {
        if (m_passes[BLOOM_PASS])
        {
            return get_pass<Core::BloomPass>(BLOOM_PASS)->get_bloom_strength();
        }
        return 0.0f;
    }
    inline void set_bloom_strength(float st) {
        if (m_passes[BLOOM_PASS])
        {
            return get_pass<Core::BloomPass>(BLOOM_PASS)->set_bloom_strength(st);
        }
    }
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif