#ifndef VK_UNIFORMS
#define VK_UNIFORMS

#include "vk_buffer.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

struct CameraUniforms
{
    Mat4 view;
    Mat4 proj;
    Mat4 viewProj;
};

struct LightUniforms
{
    Vec4 position = {0.0f, 0.0f, 0.0f, 0.0f}; // w for type
    Vec4 color = {0.0f, 0.0f, 0.0f, 0.0f};
    Vec4 dataSlot1 = {0.0f, 0.0f, 0.0f, 0.0f};
    Mat4 viewProj;
    Vec4 dataSlot2 = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct SceneUniforms
{
    Vec4 fogColor;     // w is for exponent
    Vec4 fogParams;    // x for near, y for far, z for intensity, w enable.
    Vec4 ambientColor; // w intensity
    LightUniforms lightUniforms[VK_MAX_LIGHTS];
    int numLights;
};

struct ObjectUniforms
{
    Mat4 model;
    Vec4 otherParams; // x is affected by fog, y is receive shadows, z cast shadows
};

struct MaterialUniforms
{
    Vec4 dataSlot1;
    Vec4 dataSlot2;
    Vec4 dataSlot3;
    Vec4 dataSlot4;
    Vec4 dataSlot5;
};

VULKAN_ENGINE_NAMESPACE_END

#endif