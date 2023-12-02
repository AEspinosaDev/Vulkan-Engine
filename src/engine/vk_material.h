#ifndef VK_MATERIAL
#define VK_MATERIAL

#include "internal/vk_core.h"

namespace vke
{

    class Material
    {
    protected:
        VkPipeline m_pipeline{};
        VkPipelineLayout m_pipelineLayout{};
        size_t m_pipelineID;
        bool m_pipelineAssigned{false};

        friend class Renderer;

    public:
        Material(){}
        

    };

}

#endif