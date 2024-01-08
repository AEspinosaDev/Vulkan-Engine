#ifndef VK_TEXTURE
#define VK_TEXTURE

#include <stb_image.h>
#include "../private/vk_image.h"
#include "../private/vk_descriptors.h"

namespace vke
{
    class Texture
    {
        unsigned char *m_tmpCache;

        int m_width;
        int m_height;
        int m_depth;
        int m_channels;

        Image m_image;
        VkSampler m_sampler;

        
        

        bool loaded{false};
        bool buffer_loaded{false};

        friend class Renderer;

    public:
        Texture() : m_tmpCache(nullptr), m_depth(1) {}

        inline bool is_data_loaded() const { return loaded; }
        inline bool is_buffer_loaded() const { return buffer_loaded; }

        inline int get_width() const { return m_width; }
        inline int get_height() const { return m_height; }
        inline int get_depth() const { return m_depth; }
        inline int get_num_channels() const { return m_channels; }

        bool load_image(std::string fileName);
        void create_sampler(VkDevice& device);

        void cleanup(VkDevice& device, VmaAllocator& memory);
    };
}

#endif