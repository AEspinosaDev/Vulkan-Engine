#ifndef VK_RENDERER
#define VK_RENDERER

#include "../private/vk_core.h"
#include "../private/vk_utils.h"
#include "../private/vk_bootstrap.h"
#include "../private/vk_initializers.h"
#include "../private/vk_swapchain.h"
#include "../private/vk_pipeline.h"
#include "../private/vk_frame.h"
#include "../private/vk_image.h"
#include "../private/vk_uniforms.h"
#include "../private/vk_descriptors.h"
#include "../private/vk_renderpass.h"

#include "vk_config.h"
#include "vk_window.h"
#include "vk_material.h"
#include "vk_texture.h"
#include "scene_objects/vk_scene.h"
#include "utilities/vk_gui.h"

namespace vke
{

	struct RendererSettings
	{

		AntialiasingType AAtype{MSAA_x4};
		BufferingType bufferingType{_DOUBLE};
		ShadowResolution shadowResolution{LOW};

		glm::vec4 clearColor{glm::vec4{0.0, 0.0, 0.0, 1.0}};

		bool autoClearColor{true};
		bool autoClearDepth{true};
		bool autoClearStencil{true};

		bool depthTest{true};
		bool depthWrite{true};

		bool enableUI{false};
	};
	/**
	 * Core class whose porpuse is to render data on a window.
	 */
	class Renderer
	{
#pragma region Properties

		RendererSettings m_settings{};

		struct UploadContext
		{
			VkFence uploadFence;
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;
		};

		UploadContext m_uploadContext{};

		VmaAllocator m_memory{};

		Window *m_window;
		VkInstance m_instance{};
		VkPhysicalDevice m_gpu{};
		VkDevice m_device{};
		VkQueue m_graphicsQueue{};
		VkQueue m_presentQueue{};
		VkDebugUtilsMessengerEXT m_debugMessenger{};

		Swapchain m_swapchain;

		std::vector<Frame> m_frames;

		VkRenderPass m_renderPass{};
		VkRenderPass m_shadowPass{};

		Texture *m_shadowTexture;
		VkFramebuffer m_shadowFramebuffer{};

		std::unordered_map<std::string, ShaderPass *> m_shaderPasses;

		DescriptorManager m_descriptorMng{};
		DescriptorSet m_globalDescriptor{};

		Buffer m_globalUniformsBuffer{};

		vkutils::DeletionQueue m_deletionQueue;

		const int MAX_FRAMES_IN_FLIGHT{2};
		const int MAX_OBJECTS_IN_FLIGHT{10};
		const int MAX_LIGHTS_IN_FLIGHT{10};

#ifdef NDEBUG
		const bool m_enableValidationLayers{false};
#else
		const bool m_enableValidationLayers{true};
#endif

		bool m_framebufferResized{false};
		bool m_changeInConfiguration{false};
		bool m_initialized{false};
		uint32_t m_currentFrame{0};

		Material *m_lastMaterial{nullptr};
		Geometry *m_lastGeometry{nullptr};
		GUIOverlay *m_gui{nullptr};

#pragma endregion
#pragma region Getters & Setters
	public:
		inline Window *const get_window() const { return m_window; }

		inline RendererSettings get_settings() { return m_settings; }
		inline void set_settings(RendererSettings settings) { m_settings = settings; }

		inline void set_clearcolor(glm::vec4 c)
		{
			m_settings.clearColor = c;
		}
		inline void set_antialiasing(AntialiasingType msaa)
		{
			m_settings.AAtype = msaa;
			if (m_initialized)
			{
				m_framebufferResized = true;
				m_changeInConfiguration = true;
			}
		}
		inline void set_shadow_quality(ShadowResolution quality)
		{
			m_settings.shadowResolution = quality;
			if (m_initialized)
			{
				m_framebufferResized = true;
				m_changeInConfiguration = true;
			}
		}

		inline void set_autoclear(bool clrColor, bool clrDepth = true, bool clrStencil = true)
		{
			m_settings.autoClearColor = clrColor;
			m_settings.autoClearDepth = clrDepth;
			m_settings.autoClearStencil = clrStencil;
		}

		inline void enable_depth_test(bool op) { m_settings.depthTest = op; }
		inline void enable_depth_writes(bool op) { m_settings.depthWrite = op; }

		inline void enable_gui_overlay(bool op) { m_settings.enableUI; }

		inline void set_gui_overlay(GUIOverlay *gui)
		{
			m_gui = gui;
			
		}

		inline GUIOverlay *get_gui_overlay()
		{
			return m_gui;
		}

#pragma endregion
#pragma region Core Functions
		Renderer(Window *window) : m_window(window) { m_frames.resize(MAX_FRAMES_IN_FLIGHT); }
		Renderer(Window *window, RendererSettings settings) : m_window(window), m_settings(settings) { m_frames.resize(MAX_FRAMES_IN_FLIGHT); }
		/**
		 * Inits the renderer.
		 */
		inline void init()
		{
			if (!m_window->m_initialized)
				m_window->init();

			init_vulkan();

			m_initialized = true;
		}
		/**
		 * Standalone pre-implemented render loop for the renderer. Call init before using this function
		 */
		void run(Scene *const scene);
		/**
		 * Renders a scene on the default backbuffer.
		 */
		void render(Scene *const scene);
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

		void init_renderpasses();

		void init_framebuffers();

		void init_control_objects();

		void init_descriptors();

		void init_shaderpasses();

		void init_resources();

		void recreate_swap_chain();

		void reconfigure_vulkan();

		void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

#pragma endregion
#pragma region Drawing

		void set_viewport(VkCommandBuffer &commandBuffer, VkExtent2D &extent, float minDepth = 0.0f, float maxDepth = 1.0f,
						  float x = 0.0f, float y = 0.0f, int offsetX = 0, int offsetY = 0);

		void default_pass(VkCommandBuffer &commandBuffer, uint32_t imageIndex, Scene *const scene);

		void shadow_pass(VkCommandBuffer &commandBuffer, Scene *const scene);

		void draw_geometry(VkCommandBuffer &commandBuffer, Geometry *const g);

#pragma endregion
#pragma region BufferManagement

		void upload_geometry_data(Geometry *const g);

		void upload_global_data(Scene *const scene);

		void setup_material(Material *const mat);

		void upload_texture(Texture *const t);
#pragma region gui
		void init_gui();
#pragma endregion
	};

}
#endif // VK_RENDERER