#pragma once
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm> // Necessary for std::clamp 
#include <shaderc/shaderc.hpp>

#include "vk_core.h"
#include "vk_utils.h"
#include "vk_bootstrap.h"
#include "vk_initializers.h"
#include "vk_swapchain.h"
#include "vk_pipeline.h"




struct GlobalParams {
	uint32_t								width = 800;
	uint32_t								height = 600;
	bool									initialized = false;
};




class VulkanEngine {
#pragma region Properties

	GlobalParams							m_globalParams;

	int										m_frameNumber{ 0 };
	int										m_selectedShader{ 0 };


	GLFWwindow*								m_window;
	VkExtent2D								m_windowExtent{ 1700 , 900 };

	VkInstance								m_instance;
	VkDebugUtilsMessengerEXT				m_debugMessenger;
	VkPhysicalDevice						m_gpu;
	VkDevice								m_device;

	VkSurfaceKHR							m_surface;
	VkRenderPass							m_renderPass;

	VkQueue									m_graphicsQueue;
	VkQueue									m_presentQueue;



	Swapchain								m_swapchain;

	std::vector<VkFramebuffer>				m_framebuffers;
	
	//Command
	VkCommandPool							m_commandPool;
	std::vector<VkCommandBuffer>			m_commandBuffers;
	//Syncs
	std::vector<VkSemaphore>				m_imageAvailableSemaphores;
	std::vector<VkSemaphore>				m_renderFinishedSemaphores;
	std::vector<VkFence>					m_inFlightFences;


	VkPipelineLayout						m_pipelineLayout;

	VkPipeline								m_currentPipeline;
	std::unordered_map<std::string,VkPipeline>						m_pipelines;




	vkutils::DeletionQueue					m_deletionQueue;

	uint32_t currentFrame = 0;
	const int MAX_FRAMES_IN_FLIGHT = 2;


	

#ifdef NDEBUG
	const bool m_enableValidationLayers = false;
#else
	const bool m_enableValidationLayers = true;
#endif
	

	bool framebufferResized = false;


#pragma endregion
public:
	inline void run() {

		init_window();
		init_vulkan();
		update();
		cleanup();

	}

private:
#pragma region Core Functions

	void init_window();

	void init_vulkan();

	void update();

	void draw();

	void cleanup();

#pragma endregion
#pragma region Vulkan Management

	void create_swapchain();

	void create_default_renderpass();

	void create_framebuffers();

	void init_commands();

	void create_sync_objects();

	void create_pipelines();

	void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void recreate_swap_chain();

	void cleanup_swap_chain();

#pragma endregion

#pragma region Input Management
	static void onKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

		std::cout << "key pressed";
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

#pragma endregion
};

