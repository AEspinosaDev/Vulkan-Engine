#ifndef VK_UNIFORMS_H
#define VK_UNIFORMS_H

#include "vk_buffer.h"

namespace vke
{
    struct CameraUniforms
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 viewProj;
    };

    struct SceneUniforms
    {
        glm::vec4 fogColor;  // w is for exponent
        glm::vec4 fogParams; // x for near, y for far, z for intensity, w unused.
        glm::vec4 ambientColor;
        glm::vec4 lightPosition; // w for light power
        glm::vec4 lightColor;    // w for light area of influence
    };

    struct ObjectUniforms
    {
        glm::mat4 model;
        glm::vec4 color; // w is opacity
        glm::vec4 otherParams; // x is affected by fog, y is receive shadows, z cast shadows
    };

    struct UnlitBasicMaterialUniforms
    {
        glm::vec4 color; // w is opacity
    };
    struct PhongMaterialUniforms
    {
        glm::vec4 albedo; // w is opacity
        glm::vec4 phongParams; // x for glossiness, y for exponent
    };

}
#endif