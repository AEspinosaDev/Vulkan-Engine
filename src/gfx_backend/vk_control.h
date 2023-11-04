#pragma once
#include  "vk_core.h"
#include "vk_bootstrap.h"
#include "vk_initializers.h"

namespace VKENG {
	/*
	* Struct containing command objects and sync structures and their initiation logic
	*/
	struct Command {
		int										maxFramesInFlight;
		//Command
		VkCommandPool							commandPool;
		std::vector<VkCommandBuffer>			commandBuffers;
		//Syncs
		std::vector<VkSemaphore>				imageAvailableSemaphores;
		std::vector<VkSemaphore>				renderFinishedSemaphores;
		std::vector<VkFence>					inFlightFences;

		void init(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface);
		void cleanup(VkDevice device);
	};

}