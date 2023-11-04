#pragma once
#include "../config.h"
#include "../gfx_backend/vk_core.h"
#include "../gfx_backend/vk_utils.h"
#include "../gfx_backend/vk_bootstrap.h"
#include "../gfx_backend/vk_initializers.h"
#include "../gfx_backend/vk_swapchain.h"
#include "../gfx_backend/vk_pipeline.h"
#include "../gfx_backend/vk_control.h"
#include "vk_mesh.h"


namespace VKENG {

	class Renderer {
#pragma region Properties

		struct UserParams {
			uint32_t								width{ 800 };
			uint32_t								height{ 600 };

			glm::vec4								clearColor{ glm::vec4(0.0,0.0,0.0,1.0) };

		};
		UserParams								m_params;

		GLFWwindow* m_window;
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
		VkPipeline* m_currentPipeline{ nullptr };
		VkPipelineLayout						m_pipelineLayout;
		std::unordered_map<std::string, VkPipeline>			m_pipelines;

		Command									m_cmd;

		vkutils::DeletionQueue					m_deletionQueue;


		Mesh* m_mesh{ nullptr };

		const int								MAX_FRAMES_IN_FLIGHT{ 2 };
#ifdef NDEBUG
		const bool								m_enableValidationLayers = false;
#else
		const bool								m_enableValidationLayers = true;
#endif
		bool									m_framebufferResized{ false };
		bool									m_initialized{ false };
		int										m_selectedShader{ 0 };
		uint32_t								m_currentFrame{ 0 };

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
#pragma region Vulkan API Management

		void create_swapchain();

		void create_default_renderpass();

		void create_framebuffers();

		void init_control_objects();

		void create_pipelines();

		void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void recreate_swap_chain();

		void cleanup_swap_chain();

#pragma endregion
#pragma region Drawing
		void draw_mesh(VkCommandBuffer cmd, Mesh* m);
#pragma endregion
#pragma region Input Management
		void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				m_selectedShader = m_selectedShader == 0 ? 1 : 0;
			}
		}

		void window_resize_callback(GLFWwindow* window, int width, int height) {
			m_framebufferResized = true;
		}

#pragma endregion
	};

}