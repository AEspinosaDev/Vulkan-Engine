/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDER_SETTINGS
#define RENDER_SETTINGS
#include <engine/common.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {
/*
Renderer Global Settings Data
*/
struct Settings {

    MSAASamples      samplesMSAA           = MSAASamples::x4;          // Multisampled AA (when possible)
    BufferingType    bufferingType         = BufferingType::DOUBLE;    // Buffering type (Usual: double buffering)
    SyncType         screenSync            = SyncType::MAILBOX;        // Type of display synchronization
    ColorFormatType  displayColorFormat    = SRGBA_8;                  // Color format used for presentation
    FloatPrecission  highDynamicPrecission = FloatPrecission::F16;     // HDR operations floating point precission
    FloatPrecission  depthPrecission       = FloatPrecission::F32;     // Depth operations floating point precission
    Vec4             clearColor            = Vec4{0.0, 0.0, 0.0, 1.0}; // Clear color of visible color buffer
    SoftwareAA       softwareAA            = SoftwareAA::NONE;
    ShadowResolution shadowQuality         = ShadowResolution::MEDIUM;
    bool             autoClearColor        = true;
    bool             autoClearDepth        = true;
    bool             autoClearStencil      = true;
    bool             enableUI              = false;
    bool             enableRaytracing      = true;
};

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END
#endif