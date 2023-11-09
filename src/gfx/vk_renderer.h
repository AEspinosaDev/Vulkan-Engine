#pragma once
#include "../config.h"
#include "backend/vk_core.h"
#include "backend/vk_utils.h"
#include "backend/vk_bootstrap.h"
#include "backend/vk_initializers.h"
#include "backend/vk_swapchain.h"
#include "backend/vk_pipeline.h"
#include "backend/vk_control.h"
#include "backend/vk_window.h"
#include "vk_mesh.h"



namespace vkeng {

	class Renderer {
#pragma region Properties

		struct UserParams {
			glm::vec4								clearColor{ glm::vec4{0.0,0.0,0.0,1.0} };
		};
		VmaAllocator							m_memory;
		UserParams								m_params;

		Window*									m_window;
		Swapchain								m_swapchain;
		Command									m_cmd;

		VkInstance								m_instance{};
		VkDebugUtilsMessengerEXT				m_debugMessenger{};
		VkPhysicalDevice						m_gpu{};
		VkDevice								m_device{};
		VkRenderPass							m_renderPass{};
		VkQueue									m_graphicsQueue{};
		VkQueue									m_presentQueue{};

		std::vector<VkFramebuffer>				m_framebuffers;
		VkPipeline*								m_currentPipeline{ nullptr };
		VkPipelineLayout						m_pipelineLayout{};
		std::unordered_map<std::string, VkPipeline>			m_pipelines;


		vkutils::DeletionQueue					m_deletionQueue;


		Mesh* m_mesh{ nullptr };

		const int								MAX_FRAMES_IN_FLIGHT{ 2 };
#ifdef NDEBUG
		const bool								m_enableValidationLayers{ false };
#else
		const bool								m_enableValidationLayers{ true };
#endif
		bool									m_framebufferResized{ false };
		bool									m_initialized{ false };
		int										m_selectedShader{ 0 };
		uint32_t								m_currentFrame{ 0 };

#pragma endregion
#pragma region Core Functions
	public:
		Renderer(Window* window): m_window(window) {}

		inline void init() {
			init_window();
			init_vulkan();
		}


		inline void run(std::vector<Mesh*> meshes) {
			update(meshes);
			cleanup();
		}

	private:

		void init_window();

		void init_vulkan();

		void update(std::vector<Mesh*> meshes);

		void draw(std::vector<Mesh*> meshes);

		void cleanup();

#pragma endregion
#pragma region Vulkan API Management

		void create_swapchain();

		void create_default_renderpass();

		void create_framebuffers();

		void init_control_objects();

		void create_pipelines();

		void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<Mesh*> meshes);

		void recreate_swap_chain();

		void cleanup_swap_chain();

#pragma endregion
#pragma region Drawing
		///*void upload_buffer(Mesh* m);
		void draw_mesh(Mesh* m, VkCommandBuffer commandBuffer);
		void upload_buffers(Mesh* m);
#pragma endregion
#pragma region Input Management
		void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				m_selectedShader = m_selectedShader == 0 ? 1 : 0;
			}
		}

		void window_resize_callback(GLFWwindow* window, int width, int height) {
			m_window->set_size(glm::vec2(width, height));
		}

#pragma endregion
	};

}