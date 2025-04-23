/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TONEMAPPING_PASS_H
#define TONEMAPPING_PASS_H
#include <engine/core/passes/postprocess_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

enum TonemappingType
{
    FILMIC_TONEMAP    = 0,
    REINDHART_TONEMAP = 1,
    ACES_TONEMAP      = 2,
    NO_TONEMAP        = 3,
};

/*
Tonemapping Pass.
*/
class TonemappingPass : public PostProcessPass<1, 1>
{
  protected:
    float           m_exposure = 1.0f;
    TonemappingType m_tonemap  = FILMIC_TONEMAP;

  public:
    /*

               Input Attachments:
               -
               - Color

               Output Attachments:
               -
               - Tonemapped Color

           */
    TonemappingPass(Graphics::Device*       device,
                    const PassConfig<1, 1>& config,
                    Extent2D                extent,
                    ColorFormatType         colorFormat)
        : PostProcessPass(device,
                          config,
                          extent,
                          colorFormat,
                          ENGINE_RESOURCES_PATH "shaders/misc/tonemapping.glsl",
                          "TONEMAPPING") {
    }

    inline float get_exposure() const {
        return m_exposure;
    }
    inline void set_exposure(float exposure) {
        m_exposure = exposure;
    }
    inline TonemappingType get_tonemapping_type() const {
        return m_tonemap;
    }
    inline void set_tonemapping_type(TonemappingType type) {
        m_tonemap = type;
    }

    virtual void setup_shader_passes();

    virtual void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif