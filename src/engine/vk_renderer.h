#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "../config.h"
#include "internal/vk_core.h"
#include "internal/vk_utils.h"
#include "internal/vk_bootstrap.h"
#include "internal/vk_initializers.h"
#include "internal/vk_swapchain.h"
#include "internal/vk_pipeline.h"
#include "internal/vk_frame.h"
#include "internal/vk_uniforms.h"
#include "vk_window.h"
#include "vk_mesh.h"
#include "vk_camera.h"
#include "vk_material.h"

namespace vke
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

	/**
	 * Core class whose porpuse is to render data on a window.
	 */
	class Renderer
	{
#pragma region Properties

		struct Settings
		{
			
			AntialiasingType AAtype{NONE};
			BufferingType bufferingType{_DOUBLE};

			glm::vec4 clearColor{glm::vec4{0.0, 0.0, 0.0, 1.0}};
			bool autoClearColor{true};
			bool autoClearDepth{true};
			bool autoClearStencil{true};
			bool depthTest{true};
			bool depthWrite{true};

			SceneUniforms sceneUniforms;
			Buffer sceneUniformBuffer;

		};

		VmaAllocator m_memory;
		Settings m_settings;

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
		VkDescriptorSetLayout m_globalSetLayout;
		VkDescriptorSetLayout m_objectSetLayout;
		VkDescriptorPool m_descriptorPool;

		std::vector<VkFramebuffer> m_framebuffers;
		
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

		Material* m_lastMaterial{nullptr};
		Geometry* m_lastGeometry{nullptr};

#pragma endregion
#pragma region Getters & Setters
	public:
		inline glm::vec4 get_clearcolor() const { return m_settings.clearColor; }

		inline void set_clearcolor(glm::vec4 c)
		{
			m_settings.clearColor = c;
		}

		inline Window *const get_window() const { return m_window; }

		inline void set_autoclear(bool clrColor, bool clrDepth = true, bool clrStencil = true)
		{
			m_settings.autoClearColor = clrColor;
			m_settings.autoClearDepth = clrDepth;
			m_settings.autoClearStencil = clrStencil;
		}

		inline void enable_depth_test(bool op){m_settings.depthTest=op;}
		inline void enable_depth_writes(bool op){m_settings.depthWrite=op;}

		inline void set_shader()
		{
			m_selectedShader = m_selectedShader == 0 ? 1 : 0;
		}

#pragma endregion
#pragma region Core Functions
		Renderer(Window *window) : m_window(window),
								   MAX_FRAMES_IN_FLIGHT(m_settings.bufferingType)
		{
		}
		/**
		 * Inits the renderer. Call it before using any other function related to this class.
		 */
		inline void init()
		{
			m_window->init();
			init_vulkan();
		}
		/**
		 * Standalone preimplemented render loop for the renderer.
		 */
		void run(std::vector<Mesh *> meshes, Camera *camera);
		/**
		 * Renders a scene given a camera on the default backbuffer.
		 */
		void render(std::vector<Mesh *> meshes, Camera *camera);
		// void render(Scene* scene, Camera *camera);
		/**
		 * Shut the renderer down.
		 */
		void shutdown();

	private:
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

		void init_pipeline(Material* m);

		void recreate_swap_chain();

		void cleanup_swap_chain();

#pragma endregion
#pragma region Drawing

		void render_pass(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<Mesh *> meshes, Camera *camera);

		void draw_meshes(VkCommandBuffer commandBuffer, std::vector<Mesh *> meshes);

		void draw_mesh(VkCommandBuffer commandBuffer, Mesh *m);

#pragma endregion
#pragma region BufferManagement

		void create_buffer(Buffer *buffer, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

		void upload_buffer(Buffer *buffer, const void *bufferData, size_t size);

		void upload_buffer(Buffer *buffer, const void *bufferData, size_t size,size_t offset);

		void setup_geometry_buffers(Geometry *g);

		void upload_global_uniform_buffers(Camera* camera, uint32_t* offsets);

	};

}
#endif // VK_RENDERER_H