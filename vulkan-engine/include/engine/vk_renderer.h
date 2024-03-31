/*
	This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

	MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

	////////////////////////////////////////////////////////////////////////////////////

	Implementation of this class is fragmentated in four submodules:

	* vk_renderer.cpp
	* vk_renderer_api_mgr.cpp
	* vk_renderer_data_mgr.cpp
	* vk_renderer_drawing.cpp

	////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef VK_RENDERER
#define VK_RENDERER

#include <engine/vk_common.h>

#include <engine/backend/vk_utils.h>
#include <engine/backend/vk_bootstrap.h>
#include <engine/backend/vk_initializers.h>
#include <engine/backend/vk_swapchain.h>
#include <engine/backend/vk_pipeline.h>
#include <engine/backend/vk_frame.h>
#include <engine/backend/vk_image.h>
#include <engine/backend/vk_uniforms.h>
#include <engine/backend/vk_descriptors.h>
#include <engine/backend/vk_renderpass.h>

#include <engine/vk_config.h>
#include <engine/vk_window.h>
#include <engine/vk_material.h>
#include <engine/vk_texture.h>
#include <engine/scene_objects/vk_scene.h>
#include <engine/utilities/vk_gui.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct RendererSettings
{
	RendererType renderingType{FORWARD};

	AntialiasingType AAtype{MSAA_x4};
	BufferingType bufferingType{_DOUBLE};
	SyncType screenSync{MAILBOX_SYNC};
	ColorFormatType colorFormat{SBGRA_8};
	DepthFormatType depthFormat{D32F};

	ShadowResolution shadowResolution{LOW};

	Vec4 clearColor{Vec4{0.0, 0.0, 0.0, 1.0}};

	bool autoClearColor{true};
	bool autoClearDepth{true};
	bool autoClearStencil{true};

	bool depthTest{true};
	bool depthWrite{true};

	bool enableUI{false};

	bool gammaCorrection{true};
};
/**
 * Renders a given scene data to a given window. Fully parametrizable. Main class of the library.
 */
class Renderer
{
#pragma region ____________________ Properties _____________________

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

	std::unordered_map<std::string, ShaderPass *> m_shaderPasses;

	std::unordered_map<uint32_t, RenderPass> m_renderPasses;

	DescriptorManager m_descriptorMng{};

	DescriptorSet m_globalDescriptor{};
	Buffer m_globalUniformsBuffer{};

	utils::DeletionQueue m_deletionQueue;

	const int MAX_FRAMES_IN_FLIGHT{2};

#ifdef NDEBUG
	const bool m_enableValidationLayers{false};
#else
	const bool m_enableValidationLayers{true};
#endif

	bool m_updateSwapchain{false};
	bool m_updateConfiguration{false};
	bool m_initialized{false};
	uint32_t m_currentFrame{0};

	// Material *m_lastMaterial{nullptr};
	// Geometry *m_lastGeometry{nullptr};

	GUIOverlay *m_gui{nullptr};

#pragma endregion
public:
	Renderer(Window *window) : m_window(window) { m_frames.resize(MAX_FRAMES_IN_FLIGHT); }
	Renderer(Window *window, RendererSettings settings) : m_window(window), m_settings(settings) { m_frames.resize(MAX_FRAMES_IN_FLIGHT); }

#pragma region _____________________ Getters & Setters _____________________

	inline Window *const get_window() const { return m_window; }

	inline RendererSettings get_settings() { return m_settings; }
	inline void set_settings(RendererSettings settings) { m_settings = settings; }

	inline void set_clearcolor(Vec4 c)
	{
		m_settings.clearColor = c;
	}
	inline void set_antialiasing(AntialiasingType msaa)
	{
		m_settings.AAtype = msaa;
		if (m_initialized)
		{
			m_updateSwapchain = true;
		}
	}
	inline void set_shadow_quality(ShadowResolution quality)
	{
		m_settings.shadowResolution = quality;
		if (m_initialized)
		{
			m_updateSwapchain = true;
		}
	}
	inline void set_color_format(ColorFormatType color)
	{
		m_settings.colorFormat = color;
		if (m_initialized)
			m_updateSwapchain = true;
	}
	inline void set_depth_format(DepthFormatType d)
	{
		m_settings.depthFormat = d;
		if (m_initialized)
			m_updateSwapchain = true;
	}
	inline void set_sync_type(SyncType sync)
	{
		m_settings.screenSync = sync;
		if (m_initialized)
			m_updateSwapchain = true;
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
#pragma region _____________________ Core Functions _____________________

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
/*
	////////////////////////////////////////////////////////////////////////////////////

	Implementation of this region can be found in the module ==>> vk_renderer_api_mgr.cpp

	////////////////////////////////////////////////////////////////////////////////////
*/
#pragma region _____________________ Vulkan API Management _____________________

	/*
	Init renderpasses and create framebuffers attached to them
	*/
	void init_renderpasses();

	/*
	Helper function that, given a pass, it creates the associated framebuffer and all its resources
	*/
	void create_framebuffer(RenderPass &pass, VkExtent2D extent, uint32_t layers = 1, uint32_t count = 1);

	/*
	Render flow control objects creation
	*/
	void init_control_objects();

	/*
	Descriptor pool and layouts creation
	*/
	void init_descriptors();

	/*
	Shader pass creation
	*/
	void init_shaderpasses();

	/*
	Resource like samplers, base textures and misc creation
	*/
	void init_resources();

	void clean_framebuffer(RenderPass &pass);

	void recreate_swap_chain();

	void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

#pragma endregion
/*
	////////////////////////////////////////////////////////////////////////////////////

	Implementation of this region can be found in the module ==>> vk_renderer_drawing.cpp

	////////////////////////////////////////////////////////////////////////////////////
*/
#pragma region _____________________ Drawing _____________________
	/*
	Record a viewport resize to the command buffer
	*/
	void set_viewport(VkCommandBuffer &commandBuffer, VkExtent2D extent, float minDepth = 0.0f, float maxDepth = 1.0f,
					  float x = 0.0f, float y = 0.0f, int offsetX = 0, int offsetY = 0);
	/*
	Forward rendering
	*/
	void render_forward(VkCommandBuffer &commandBuffer, uint32_t imageIndex, Scene *const scene);
	/*
	Deferred rendering
	*/
	void render_deferred(VkCommandBuffer &commandBuffer, uint32_t imageIndex, Scene *const scene);
	/*
	Default forward pass
	*/
	void forward_pass(VkCommandBuffer &commandBuffer, uint32_t imageIndex, Scene *const scene);
	/*
	Pass tom save depth values from ligh view and use it for shadow mapping
	*/
	void shadow_pass(VkCommandBuffer &commandBuffer, Scene *const scene);
	/*
	Geometry pass. Used for deferred rendering.
	*/
	void geometry_pass(VkCommandBuffer &commandBuffer, Scene *const scene);
	/*
	Lighting pass. Used for deferred rendering.
	*/
	void lighting_pass(VkCommandBuffer &commandBuffer, Scene *const scene);
	/*
	Render single geometry
	*/
	void draw_geometry(VkCommandBuffer &commandBuffer, Geometry *const g);

#pragma endregion
/*
	////////////////////////////////////////////////////////////////////////////////////

	Implementation of this region can be found in the module ==>> vk_renderer_data_mgr.cpp

	////////////////////////////////////////////////////////////////////////////////////
*/
#pragma region _____________________ Data Management _____________________
	/*
	Geometry vertex and index buffers upload to GPU
	*/
	void upload_geometry_data(Geometry *const g);
	/*
	Object descriptor layouts uniforms buffer upload to GPU
	*/
	void upload_object_data(Scene *const scene);
	/*
	Global descriptor layouts uniforms buffer upload to GPU
	*/
	void upload_global_data(Scene *const scene);

	/*
	Initialize and setup textures and uniforms in given material
	*/
	void setup_material(Material *const mat);
	/*
	Texture setup and upload to GPU. Mipmap and sampler creation
	*/
	void upload_texture(Texture *const t);
#pragma region _____________________ GUI _____________________
	/*
	Initialize gui layout in case ther's one enabled
	*/
	void init_gui();

#pragma endregion
};

VULKAN_ENGINE_NAMESPACE_END

#endif // VK_RENDERER