#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Image::create_view(ImageConfig config) {
    VkImageViewCreateInfo dview_info = Init::imageview_create_info(Translator::get(config.format),
                                                                   handle,
                                                                   Translator::get(config.viewType),
                                                                   Translator::get(config.aspectFlags),
                                                                   mipLevels,
                                                                   layers,
                                                                   baseMipLevel);
    VK_CHECK(vkCreateImageView(device, &dview_info, nullptr, &view));
}
void Image::create_sampler(SamplerConfig config) {
    VkSamplerCreateInfo samplerInfo = Init::sampler_create_info(Translator::get(config.filters),
                                                                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                                                                config.minLod,
                                                                config.maxLod <= mipLevels ? config.maxLod : mipLevels+baseMipLevel,
                                                                config.anysotropicFilter,
                                                                config.maxAnysotropy,
                                                                Translator::get(config.samplerAddressMode));
    samplerInfo.borderColor         = Translator::get(config.border);

    VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
}

void Image::create_GUI_handle() {
    if (GUIReadHandle == VK_NULL_HANDLE)
        GUIReadHandle =
            ImGui_ImplVulkan_AddTexture(sampler, view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
void Image::upload_image(VkCommandBuffer& cmd, Buffer* stagingBuffer) {

    VkImageSubresourceRange range;
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel   = 0;
    range.levelCount     = mipLevels;
    range.baseArrayLayer = 0;
    range.layerCount     = layers;

    VkImageMemoryBarrier imageBarrier_toTransfer = {};
    imageBarrier_toTransfer.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    imageBarrier_toTransfer.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier_toTransfer.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toTransfer.image            = handle;
    imageBarrier_toTransfer.subresourceRange = range;

    imageBarrier_toTransfer.srcAccessMask = 0;
    imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // barrier the image into the transfer-receive layout
    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &imageBarrier_toTransfer);

    // For each layer
    for (uint32_t layer = 0; layer < layers; ++layer)
    {
        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset =
            layer * ((extent.width * extent.height * stagingBuffer->size) / layers); // Offset per face
        copyRegion.bufferRowLength   = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel       = 0;
        copyRegion.imageSubresource.baseArrayLayer = layer;
        copyRegion.imageSubresource.layerCount     = 1;
        copyRegion.imageExtent                     = extent;

        vkCmdCopyBufferToImage(
            cmd, stagingBuffer->handle, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    }

    if (mipLevels == 1)
    {
        VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

        imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // barrier the image into the shader readable layout
        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &imageBarrier_toReadable);
    }
}
void Image::generate_mipmaps(VkCommandBuffer& cmd) {

    int32_t mipWidth  = extent.width;
    int32_t mipHeight = extent.height;
    int32_t mipDepth  = extent.depth;

    VkImageSubresourceRange range;
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    range.levelCount     = 1;
    range.baseArrayLayer = 0;
    range.layerCount     = layers;

    VkImageMemoryBarrier imageBarrier_toTransfer = {};
    imageBarrier_toTransfer.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier_toTransfer.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier_toTransfer.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier_toTransfer.image                = handle;
    imageBarrier_toTransfer.subresourceRange     = range;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        for (uint32_t layer = 0; layer < layers; ++layer)
        {
            // Set barriers for the current face
            imageBarrier_toTransfer.subresourceRange.baseMipLevel   = i - 1;
            imageBarrier_toTransfer.subresourceRange.baseArrayLayer = layer;
            imageBarrier_toTransfer.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toTransfer.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageBarrier_toTransfer.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toTransfer.dstAccessMask                   = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(cmd,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imageBarrier_toTransfer);

            VkImageBlit blit{};
            blit.srcOffsets[0]                 = {0, 0, 0};
            blit.srcOffsets[1]                 = {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel       = i - 1;
            blit.srcSubresource.baseArrayLayer = layer; // Specify the current face
            blit.srcSubresource.layerCount     = 1;

            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {
                mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
            blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel       = i;
            blit.dstSubresource.baseArrayLayer = layer; // Specify the current face
            blit.dstSubresource.layerCount     = 1;

            vkCmdBlitImage(cmd,
                           handle,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           handle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blit,
                           VK_FILTER_LINEAR);
        }

        imageBarrier_toTransfer.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageBarrier_toTransfer.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrier_toTransfer.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &imageBarrier_toTransfer);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
        if (mipDepth > 1)
            mipDepth /= 2;
    }

    VkImageMemoryBarrier imageBarrier_toReadable          = imageBarrier_toTransfer;
    imageBarrier_toReadable.subresourceRange.baseMipLevel = mipLevels - 1;
    imageBarrier_toReadable.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toReadable.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBarrier_toReadable.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier_toReadable.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &imageBarrier_toReadable);
}

void Image::cleanup(bool destroySampler) {
    if (view)
    {
        vkDestroyImageView(device, view, nullptr);
        view = VK_NULL_HANDLE;
    }
    if (handle && memory)
    {
        vmaDestroyImage(memory, handle, allocation);
        handle = VK_NULL_HANDLE;
    }
    if (destroySampler && sampler)
    {
        vkDestroySampler(device, sampler, VK_NULL_HANDLE);
        sampler = VK_NULL_HANDLE;
    }
    if (GUIReadHandle)
    {
        ImGui_ImplVulkan_RemoveTexture(GUIReadHandle);
        GUIReadHandle = VK_NULL_HANDLE;
    }
}

Image Image::clone() const {
    Image img         = {};
    img.handle        = handle;
    img.device        = device;
    img.memory        = memory;
    img.allocation    = allocation;
    img.view          = view;
    img.sampler       = sampler;
    img.GUIReadHandle = GUIReadHandle;

    img.extent       = extent;
    img.layers       = layers;
    img.mipLevels    = mipLevels;
    img.baseMipLevel = baseMipLevel;

    return img;
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END