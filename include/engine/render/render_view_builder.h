/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDER_VIEW_BUILDER
#define RENDER_VIEW_BUILDER

#include <engine/graphics/device.h>

#include <engine/render/frame.h>
#include <engine/render/render_resources.h>
#include <engine/render/render_settings.h>
#include <engine/render/render_view.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

class RenderViewBuilder
{
    std::string m_global_UBO_Key;
    std::string m_object_UBO_Key;

public:
    inline void set_keys( const std::string& globalUBO_key, const std::string& objectUBO_key ) {
        m_global_UBO_Key = globalUBO_key;
        m_object_UBO_Key = objectUBO_key;
    }

    RenderView build( const ptr<Graphics::Device>& device, Render::Frame* const currentFrame, Core::Scene* const scene, Extent2D displayExtent, const Settings& settings );

private:
    void update_global_data( const ptr<Graphics::Device>& device,
                             Render::Frame* const         currentFrame,
                             Core::Scene* const           scene,
                             Extent2D                     displayExtent,
                             bool                         jitterCamera,
                             RenderView&                  view );
    void update_object_data( const ptr<Graphics::Device>& device,
                             Render::Frame* const         currentFrame,
                             Core::Scene* const           scene,
                             Extent2D                     displayExtent,
                             bool                         enableRT,
                             RenderView&                  view );
};
} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif