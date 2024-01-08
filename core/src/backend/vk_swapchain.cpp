#include "vk_swapchain.h"
namespace vke
{

	void Swapchain::create(VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface, GLFWwindow *window, VkExtent2D &windowExtent)
	{
		vkboot::SwapChainSupportDetails swapChainSupport = vkboot::query_swapchain_support(gpu, surface);
		vkboot::QueueFamilyIndices indices = vkboot::find_queue_families(gpu, surface);
		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
		VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
		VkExtent2D extent = choose_swap_extent(swapChainSupport.capabilities, window);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

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
			createInfo.queueFamilyIndexCount = 0;	  // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain));

		VK_CHECK(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr);

				 m_presentImages.resize(imageCount));

		VK_CHECK(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_presentImages.data()));

		m_presentFormat = surfaceFormat.format;
		windowExtent = extent;

		create_image_views(device);
	}

	void Swapchain::create_colorbuffer(VkDevice &device, VmaAllocator &memory, VkExtent2D &windowExtent, VkSampleCountFlagBits samples)
	{

		VkExtent3D bufferExtent = {
			windowExtent.width,
			windowExtent.height,
			1};

		VmaAllocationCreateInfo cimg_allocinfo = {};
		cimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		cimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_colorBuffer.init(memory, m_presentFormat, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, cimg_allocinfo, bufferExtent, samples);
		m_colorBuffer.create_view(device, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void Swapchain::create_depthbuffer(VkDevice &device, VmaAllocator &memory, VkExtent2D &windowExtent, VkSampleCountFlagBits samples)
	{
		VkExtent3D bufferExtent = {
			windowExtent.width,
			windowExtent.height,
			1};

		VmaAllocationCreateInfo dimg_allocinfo = {};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_depthBuffer.init(memory, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, dimg_allocinfo, bufferExtent, samples);
		m_depthBuffer.create_view(device, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void Swapchain::create_framebuffers(VkDevice &device, VkRenderPass &defaultRenderPass, VkExtent2D &windowExtent, VkSampleCountFlagBits samples)
	{
		auto size = m_presentImageViews.size();
		m_framebuffers.resize(size);

		VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(defaultRenderPass, windowExtent);
		for (size_t i = 0; i < size; i++)
		{
			if (samples != VK_SAMPLE_COUNT_1_BIT)
			{
				VkImageView attachments[3] = {
					m_colorBuffer.view,
					m_depthBuffer.view,
					m_presentImageViews[i]};
				fb_info.pAttachments = attachments;
				fb_info.attachmentCount = 3;
			}
			else
			{
				VkImageView attachments[2] = {
					m_presentImageViews[i],
					m_depthBuffer.view};
				fb_info.pAttachments = attachments;
				fb_info.attachmentCount = 2;
			}

			if (vkCreateFramebuffer(device, &fb_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void Swapchain::cleanup(VkDevice &device, VmaAllocator &memory)
	{
		for (size_t i = 0; i < m_framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(device, m_framebuffers[i], nullptr);
		}
		for (size_t i = 0; i < m_presentImageViews.size(); i++)
		{
			vkDestroyImageView(device, m_presentImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device, m_swapchain, nullptr);

		m_colorBuffer.cleanup(device, memory);
		m_depthBuffer.cleanup(device, memory);
	}

	VkSurfaceFormatKHR Swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats)
	{
		for (const auto &availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR> &availablePresentModes)
	{
		for (const auto &availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void Swapchain::create_image_views(VkDevice &device)
	{
		m_presentImageViews.resize(m_presentImages.size());

		for (size_t i = 0; i < m_presentImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_presentImages[i];
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

			VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &m_presentImageViews[i]));
		}
	}

}