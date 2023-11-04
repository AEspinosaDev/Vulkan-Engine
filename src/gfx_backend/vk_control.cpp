#include "vk_control.h"
namespace VKENG {

	void Command::init(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface)
	{
		//create a command pool for commands submitted to the graphics queue.
		//we also want the pool to allow for resetting of individual command buffers
		VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(vkboot::find_queue_families(gpu, surface).graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

		//allocate the default command buffer that we will use for rendering
		commandBuffers.resize(maxFramesInFlight);
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(commandPool, (uint32_t)commandBuffers.size());

		VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, commandBuffers.data()));

		imageAvailableSemaphores.resize(maxFramesInFlight);
		renderFinishedSemaphores.resize(maxFramesInFlight);
		inFlightFences.resize(maxFramesInFlight);

		//create syncronization structures
		//one fence to control when the gpu has finished rendering the frame,
		//and 2 semaphores to syncronize rendering with swapchain
		//we want the fence to start signalled so we can wait on it on the first frame
		VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();
		VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

		for (size_t i = 0; i < maxFramesInFlight; i++) {

			VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]));
			VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]));

		}

	}

	void Command::cleanup(VkDevice device)
	{
		vkDestroyCommandPool(device, commandPool, nullptr);
		for (size_t i = 0; i < maxFramesInFlight; i++) {

			vkDestroyFence(device, inFlightFences[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);

		}


	}

}