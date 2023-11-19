#pragma once
#include "vk_buffer.h"

namespace vkeng
{
    struct CameraUniforms
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 viewProj;
    };

}