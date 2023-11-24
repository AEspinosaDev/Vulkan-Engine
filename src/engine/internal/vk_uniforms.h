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

}
#endif