
#include "vk_image.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

void Image::init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples)
{
    extent = imageExtent;

    format = imageFormat;

    mipLevels = useMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(imageExtent.width, imageExtent.height)))) + 1 : 1;

    VkImageCreateInfo img_info = vkinit::image_create_info(format, usageFlags, extent, mipLevels, samples);

    VmaAllocationCreateInfo img_allocinfo = {};
    img_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // allocate and create the image
    vmaCreateImage(memory, &img_info, &img_allocinfo, &image, &allocation, nullptr);
}

void Image::init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VmaAllocationCreateInfo &allocInfo, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples)
{
    extent = imageExtent;

    format = imageFormat;

    mipLevels = useMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(imageExtent.width, imageExtent.height)))) + 1 : 1;

    VkImageCreateInfo img_info = vkinit::image_create_info(format, usageFlags, extent, mipLevels, samples);

    vmaCreateImage(memory, &img_info, &allocInfo, &image, &allocation, nullptr);
}

void Image::upload_image(VkCommandBuffer &cmd, Buffer *stagingBuffer)
{

    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = mipLevels;
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

    if (mipLevels == 1)
    {
        VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

        imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // barrier the image into the shader readable layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
    }
}
void Image::generate_mipmaps(VkCommandBuffer &cmd)
{

    int32_t mipWidth = extent.width;
    int32_t mipHeight = extent.height;

    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    VkImageMemoryBarrier imageBarrier_toTransfer = {};
    imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier_toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier_toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier_toTransfer.image = image;
    imageBarrier_toTransfer.subresourceRange = range;

    for (uint32_t i = 1; i < mipLevels; i++)
    {

        imageBarrier_toTransfer.subresourceRange.baseMipLevel = i - 1;
        imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageBarrier_toTransfer.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imageBarrier_toTransfer);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(cmd,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrier_toTransfer.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
    imageBarrier_toReadable.subresourceRange.baseMipLevel = mipLevels - 1;
    imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &imageBarrier_toReadable);
}
void Image::create_view(VkDevice &device, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(format, image, aspectFlags, mipLevels);
    VK_CHECK(vkCreateImageView(device, &dview_info, nullptr, &view));
}
void Image::cleanup(VkDevice &device, VmaAllocator &memory)
{
    vkDestroyImageView(device, view, nullptr);
    vmaDestroyImage(memory, image, allocation);
}
VULKAN_ENGINE_NAMESPACE_END