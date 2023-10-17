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


//Shader class
//Pipeline class
// 
//SwapChain class

struct GlobalParams {
	uint32_t								width = 800;
	uint32_t								height = 600;
	bool									initialized = false;
};



struct ShaderSource
{
	std::string								vert;
	std::string								frag;
	std::string								geom;
	std::string								tess;
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


	VkCommandPool							m_commandPool;
	std::vector<VkCommandBuffer>			m_commandBuffers;



	Swapchain								m_swapchain;

	std::vector<VkFramebuffer>				m_framebuffers;
	

	VkPipelineLayout						m_pipelineLayout;
	VkPipeline								m_graphicsPipeline;


	vkutils::DeletionQueue					m_deletionQueue;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	//VkPipelineLayout _trianglePipelineLayout;
	//VkPipeline _trianglePipeline;
	//VkPipeline _redTrianglePipeline;

	/*DeletionQueue _mainDeletionQueue;*/

	//friend class vkboot::VulkanBooter;

#ifdef NDEBUG
	const bool m_enableValidationLayers = false;
#else
	const bool m_enableValidationLayers = true;
#endif
	

	bool framebufferResized = false;


#pragma endregion
public:
	void run() {

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

	void draw_frame();

	void cleanup();

#pragma endregion
#pragma region Vulkan Management
	//Initiation
	void create_swapchain();
	void create_default_renderpass();
	void create_framebuffers();
	//void init_commands();
	//void create_pipelines();

	void createGraphicPipeline();
	void createCommandPool();
	void create_command_buffers();
	void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);


	void create_sync_objects();

	void recreate_swap_chain();

	void cleanup_swap_chain();



	



	std::vector<char> readFile(const std::string& filename);
	ShaderSource readShaderFile(const std::string& filePath);

	VkShaderModule createShaderModule(const std::vector<uint32_t> code);
	std::vector<uint32_t> compileShader(const std::string src, const std::string shaderName, shaderc_shader_kind kind, bool optimize);

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}
#pragma endregion

};

