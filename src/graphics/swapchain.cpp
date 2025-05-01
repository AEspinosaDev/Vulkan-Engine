#include <engine/graphics/swapchain.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

VkExtent2D Swapchain::create_surface(VkInstance instance, void* windowHandle, WindowingSystem windowingSystem) {
    VkExtent2D actualExtent{};
    if (windowingSystem == WindowingSystem::GLFW)
    {
        GLFWwindow* glfwHandle = static_cast<GLFWwindow*>(windowHandle);
        VK_CHECK(glfwCreateWindowSurface(instance, glfwHandle, nullptr, &m_surface));
        int width, height;
        glfwGetFramebufferSize(glfwHandle, &width, &height);
        actualExtent = {static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
    } else
    {
        // TO DO SDL ..
    }

    return actualExtent;
}

void Swapchain::create(VkPhysicalDevice& gpu,
                       VkDevice&         device,
                       VkExtent2D        actualExtent,
                       VkExtent2D        windowExtent,
                       uint32_t          imageCount,
                       VkFormat          userDefinedcolorFormat,
                       VkPresentModeKHR  userDefinedPresentMode) {
    m_device = device;
    if (m_initialized)
        cleanup();

    Booter::SwapChainSupportDetails swapChainSupport     = Booter::query_swapchain_support(gpu, m_surface);
    Booter::QueueFamilyIndices      indices              = Booter::find_queue_families(gpu, m_surface);
    uint32_t                        queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats, userDefinedcolorFormat);
    VkPresentModeKHR   presentMode   = choose_swap_present_mode(swapChainSupport.presentModes, userDefinedPresentMode);
    VkExtent2D         extent        = choose_swap_extent(swapChainSupport.capabilities, actualExtent);

    imageCount = swapChainSupport.capabilities.minImageCount + (imageCount - 1);

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;       // Optional
        createInfo.pQueueFamilyIndices   = nullptr; // Optional
    }
    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_handle));

    std::vector<VkImage> images;
    images.resize(imageCount);

    VK_CHECK(vkGetSwapchainImagesKHR(device, m_handle, &imageCount, images.data()));

    m_presentImages.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        m_presentImages[i].handle = images[i];
        m_presentImages[i].device = device;
    }

    m_presentFormat = surfaceFormat.format;
    m_presentMode   = presentMode;
    windowExtent    = extent;

    create_image_views(device);

    m_initialized = true;
}

void Swapchain::destroy_surface(VkInstance instance) {
    if (m_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
}
void Swapchain::cleanup() {
    for (size_t i = 0; i < m_presentImages.size(); i++)
    {
        if (m_presentImages[i].view != VK_NULL_HANDLE)
            vkDestroyImageView(m_device, m_presentImages[i].view, nullptr);
    }

    if (m_handle != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(m_device, m_handle, nullptr);

    m_initialized = false;
}

VkSurfaceFormatKHR Swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats, VkFormat desiredFormat) {
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == desiredFormat)
        {
            return availableFormat;
        }
    }

    // Fallback
    return availableFormats[0];
}

VkPresentModeKHR Swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR desiredMode) {
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == desiredMode)
        {
            return availablePresentMode;
        }
    }

    // Fallback
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D actualExtent) {
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
    {
        return capabilities.currentExtent;
    } else
    {

        actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Swapchain::create_image_views(VkDevice& device) {
    for (size_t i = 0; i < m_presentImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                           = m_presentImages[i].handle;
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = m_presentFormat;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &m_presentImages[i].view));
    }
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END