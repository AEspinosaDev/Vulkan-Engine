
#include "vk_image.h"

namespace vke
{
    void Image::init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VkExtent3D imageExtent)
    {
        extent = imageExtent;

        format = imageFormat;

        VkImageCreateInfo img_info = vkinit::image_create_info(format, usageFlags, extent);

        VmaAllocationCreateInfo img_allocinfo = {};
        img_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        // allocate and create the image
        vmaCreateImage(memory, &img_info, &img_allocinfo, &image, &allocation, nullptr);
    }

    void Image::init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VmaAllocationCreateInfo &allocInfo, VkExtent3D imageExtent)
    {
        extent = imageExtent;

        format = imageFormat;

        VkImageCreateInfo img_info = vkinit::image_create_info(format, usageFlags, extent);

        vmaCreateImage(memory, &img_info, &allocInfo, &image, &allocation, nullptr);
    }

    void Image::upload_image(VkCommandBuffer cmd, Buffer *stagingBuffer)
    {
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier imageBarrier_toTransfer = {};
        imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier_toTransfer.image = image;
        imageBarrier_toTransfer.subresourceRange = range;

        imageBarrier_toTransfer.srcAccessMask = 0;
        imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // barrier the image into the transfer-receive layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = extent;

        // copy the buffer into the image
        vkCmdCopyBufferToImage(cmd, stagingBuffer->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

        imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // barrier the image into the shader readable layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
    }
    void Image::create_view(VkDevice device, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(format, image, aspectFlags);
        VK_CHECK(vkCreateImageView(device, &dview_info, nullptr, &view));
    }
    void Image::cleanup(VkDevice device, VmaAllocator memory)
    {
        vkDestroyImageView(device, view, nullptr);
        vmaDestroyImage(memory, image, allocation);
    }
}