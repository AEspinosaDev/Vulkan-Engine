#pragma once
#include "../config.h"
#include "backend/vk_core.h"
#include "backend/vk_utils.h"
#include "backend/vk_bootstrap.h"
#include "backend/vk_initializers.h"
#include "backend/vk_swapchain.h"
#include "backend/vk_pipeline.h"
#include "backend/vk_frame.h"
#include "backend/vk_uniforms.h"
#include "vk_window.h"
#include "vk_mesh.h"
#include "vk_camera.h"

namespace vkeng
{
	enum BufferingType
	{
		_UNIQUE = 1,
		_DOUBLE = 2,
		_TRIPLE = 3,
		_QUADRUPLE = 4
	};

	enum AntialiasingType
	{
		NONE = 1,
		MSAA_4 = 4,
		MSAA_8 = 8,
		MSAA_16 = 16,
	};

	class Renderer
	{
#pragma region Properties

		struct UserParams
		{
			AntialiasingType AAtype{NONE};
			BufferingType bufferingType{_DOUBLE};
			glm::vec4 clearColor{glm::vec4{0.0, 0.0, 0.0, 1.0}};
		};

		VmaAllocator m_memory;
		UserParams m_params;

		Window *m_window;
		VkInstance m_instance{};
		Swapchain m_swapchain;
		VkPhysicalDevice m_gpu{};
		VkDevice m_device{};
		VkRenderPass m_renderPass{};
		VkQueue m_graphicsQueue{};
		VkQueue m_presentQueue{};
		VkDebugUtilsMessengerEXT m_debugMessenger{};

		const int MAX_FRAMES_IN_FLIGHT;
		Frame m_frames[2];
		///
		VkDescriptorSetLayout m_globalSetLayout;
		VkDescriptorPool m_descriptorPool;

		std::vector<VkFramebuffer> m_framebuffers;
		VkPipeline *m_currentPipeline{nullptr};
		VkPipelineLayout m_pipelineLayout{};
		VkPipelineLayout m_meshPipelineLayout{};
		std::unordered_map<std::string, VkPipeline> m_pipelines;

		vkutils::DeletionQueue m_deletionQueue;

#ifdef NDEBUG
		const bool m_enableValidationLayers{false};
#else
		const bool m_enableValidationLayers{true};
#endif
		bool m_framebufferResized{false};
		bool m_initialized{false};
		int m_selectedShader{0};
		uint32_t m_currentFrame{0};

#pragma endregion
#pragma region Core Functions
	public:
		Renderer(Window *window) : m_window(window),
								   MAX_FRAMES_IN_FLIGHT(m_params.bufferingType)
		{
		}

		inline void init()
		{
			init_window();
			init_vulkan();
		}

		void run(std::vector<Mesh *> meshes, Camera *camera);

		void render(std::vector<Mesh *> meshes, Camera *camera);

	private:
		void init_window();

		void init_vulkan();

		void cleanup();

#pragma endregion
#pragma region Vulkan API Management

		void create_swapchain();

		void init_default_renderpass();

		void init_framebuffers();

		void init_control_objects();

		void init_descriptors();

		void init_pipelines();

		void recreate_swap_chain();

		void cleanup_swap_chain();

#pragma endregion
#pragma region Drawing

		void render_pass(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<Mesh *> meshes, Camera *camera);

		void draw_meshes(VkCommandBuffer commandBuffer, std::vector<Mesh *> meshes);

		void draw_mesh(VkCommandBuffer commandBuffer, Mesh *m);

#pragma endregion
#pragma region BufferManagement

		void touch_buffer();

		void upload_buffer(Buffer *buffer, const void *bufferData, size_t size);

		void create_buffer(Buffer *buffer, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

#pragma region Input Management

		void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
		{

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				m_selectedShader = m_selectedShader == 0 ? 1 : 0;
			}

			if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
			{
				m_window->set_fullscreen(m_window->is_fullscreen() ? false : true);
			}
		}

		void window_resize_callback(GLFWwindow *window, int width, int height)
		{
			m_window->set_size(width, height);
		}

#pragma endregion
	};

}