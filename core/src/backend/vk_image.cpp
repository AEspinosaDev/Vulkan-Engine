#define STB_IMAGE_IMPLEMENTATION
#include "vk_image.h"

namespace vke
{
    bool vke::Image::load_image(const char *file)
    {
        int texWidth, texHeight, texChannels;

        stbi_uc *pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            DEBUG_LOG("Failed to load texture file");
            return false;
        }

        void *pixel_ptr = pixels;
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        // Important
        //  the format R8G8B8A8 matches exactly with the pixels loaded from stb_image lib
        VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

        // Buffer tmpBuffer;
        // tmpBuffer.init(m_memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        // tmpBuffer.upload_data(m_memory, pixel_ptr, static_cast<size_t>(imageSize));

        // stbi_image_free(pixels);

        // VkExtent3D imageExtent;
        // imageExtent.width = static_cast<uint32_t>(texWidth);
        // imageExtent.height = static_cast<uint32_t>(texHeight);
        // imageExtent.depth = 1;

        // VkImageCreateInfo dimg_info = vkinit::image_create_info(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);



        // VmaAllocationCreateInfo dimg_allocinfo = {};
        // dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        // // allocate and create the image
        // vmaCreateImage(m_memory, &dimg_info, &dimg_allocinfo, &image, &allocation, nullptr);

        return true;
    }
}