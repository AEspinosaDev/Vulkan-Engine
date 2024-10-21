#include <engine/graphics/swapchain.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

void Swapchain::create(VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR surface, VkExtent2D actualExtent,
                       VkExtent2D windowExtent, uint32_t imageCount, VkFormat userDefinedcolorFormat,
                       VkPresentModeKHR userDefinedPresentMode)
{
    if (m_initialized)
        cleanup(device);

    SwapChainSupportDetails swapChainSupport = query_swapchain_support(gpu, surface);
    QueueFamilyIndices indices = find_queue_families(gpu, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats, userDefinedcolorFormat);
    VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes, userDefinedPresentMode);
    VkExtent2D extent = choose_swap_extent(swapChainSupport.capabilities, actualExtent);

    imageCount = swapChainSupport.capabilities.minImageCount + (imageCount - 1);

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain));

    std::vector<VkImage> images;
    images.resize(imageCount);

    VK_CHECK(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, images.data()));

    m_presentImages.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        m_presentImages[i].handle = images[i];
    }

    m_presentFormat = surfaceFormat.format;
    m_presentMode = presentMode;
    windowExtent = extent;

    create_image_views(device);

    m_initialized = true;
}

void Swapchain::cleanup(VkDevice &device)
{
    for (size_t i = 0; i < m_presentImages.size(); i++)
    {
        vkDestroyImageView(device, m_presentImages[i].view, nullptr);
    }

    vkDestroySwapchainKHR(device, m_swapchain, nullptr);

    m_initialized = false;
}

VkSurfaceFormatKHR Swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats,
                                                         VkFormat desiredFormat)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == desiredFormat && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // Fallback
    return availableFormats[0];
}

VkPresentModeKHR Swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR> &availablePresentModes,
                                                     VkPresentModeKHR desiredMode)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == desiredMode)
        {
            return availablePresentMode;
        }
    }

    // Fallback
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D actualExtent)
{
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
    {
        return capabilities.currentExtent;
    }
    else
    {

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Swapchain::create_image_views(VkDevice &device)
{
    for (size_t i = 0; i < m_presentImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_presentImages[i].handle;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_presentFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &m_presentImages[i].view));
    }
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END