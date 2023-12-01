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
        glm::vec4 fogColor;     // w is for exponent
        glm::vec4 fogDistances; // x for min, y for max, zw unused.
        glm::vec4 ambientColor;
        glm::vec4 sunlightDirection; // w for sun power
        glm::vec4 sunlightColor;
    };

}
#endif