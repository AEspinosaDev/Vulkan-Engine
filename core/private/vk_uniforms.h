#ifndef VK_UNIFORMS
#define VK_UNIFORMS

#include "vk_buffer.h"

namespace vke
{
    struct CameraUniforms
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 viewProj;
    };

    struct LightUniforms
    {
        glm::vec4 position = {0.0f, 0.0f, 0.0f, 0.0f}; // w for type
        glm::vec4 color = {0.0f, 0.0f, 0.0f, 0.0f};
        glm::vec4 dataSlot = {0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct SceneUniforms
    {
        glm::vec4 fogColor;     // w is for exponent
        glm::vec4 fogParams;    // x for near, y for far, z for intensity, w unused.
        glm::vec4 ambientColor; // w intensity
        LightUniforms lightUniforms;
    };

    struct ObjectUniforms
    {
        glm::mat4 model;
        glm::vec4 otherParams; // x is affected by fog, y is receive shadows, z cast shadows
    };

    struct MaterialUniforms
    {
        glm::vec4 dataSlot1;
        glm::vec4 dataSlot2;
        glm::vec4 dataSlot3;
        glm::vec4 dataSlot4;
        glm::vec4 dataSlot5;
    };

}
#endif