#ifndef DEFERRED2_H
#define DEFERRED2_H

#include <engine/systems/renderers/renderer2.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renders a given scene data to a given window using deferred rendering. Fully parametrizable.
*/
class DeferredRenderer2 : public BaseRenderer2
{
protected:
    virtual void init_resources() override;
    virtual void register_shaders() override;
    virtual void configure_passes() override;
    virtual void on_before_render( Core::Scene* const scene ) override;

public:
    DeferredRenderer2( const ptr<Core::IWindow>& window )
        : BaseRenderer2( window ) {
    }
    DeferredRenderer2( const ptr<Core::IWindow>& window, Render::Settings settings = {} )
        : BaseRenderer2( window, settings ) {
    }
    // Headless instantiation
    DeferredRenderer2( Extent2D displayExtent = { 800, 800 } )
        : BaseRenderer2( displayExtent ) {
    }
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif