/*
	This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

	MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

	////////////////////////////////////////////////////////////////////////////////////

	Implementation of this class is fragmentated in three submodules:

	* vk_renderer.cpp
	* vk_renderer_api_mgr.cpp
	* vk_renderer_data_mgr.cpp

	////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef RENDERER_H
#define RENDERER_H

#include <engine/common.h>

#include <engine/backend/utils.h>
#include <engine/backend/bootstrap.h>
#include <engine/backend/initializers.h>
#include <engine/backend/swapchain.h>
#include <engine/backend/pipeline.h>
#include <engine/backend/frame.h>
#include <engine/backend/image.h>
#include <engine/backend/uniforms.h>
#include <engine/backend/descriptors.h>

#include <engine/core/window.h>
#include <engine/core/material.h>
#include <engine/core/texture.h>
#include <engine/core/renderpass.h>

#include <engine/renderpasses/forwardpass.h>
#include <engine/renderpasses/shadowpass.h>
#include <engine/renderpasses/guipass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

/*
Renderer Global Settings Data
*/
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

	bool enableHardwareDepthBias{false};
	float hardwareDepthBias{0.0005f};

	bool gammaCorrection{true};
};
/*
Structure that contains a list of renderpasses. These renderpasses will render in the order they where added.
*/
struct RenderPipeline
{
	std::vector<RenderPass *> renderpasses;

	void push_renderpass(RenderPass *pass)
	{
		renderpasses.push_back(pass);
	};
};

/**
 * Renders a given scene data to a given window. Fully parametrizable. Main class of the library. It can be inherited for achieving a higher end application.
 */
class Renderer
{
#pragma region Properties
protected:
	RendererSettings m_settings{};

	struct UploadContext
	{
		VkFence uploadFence;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
	};

	UploadContext m_uploadContext{};

	Window *m_window;

	VkInstance m_instance{};
	VkPhysicalDevice m_gpu{};
	VkDevice m_device{};
	VmaAllocator m_memory{};
	VkQueue m_graphicsQueue{};
	VkQueue m_presentQueue{};
	VkDebugUtilsMessengerEXT m_debugMessenger{};

	Swapchain m_swapchain;

	std::vector<Frame> m_frames;

	RenderPipeline m_pipeline;

	DescriptorManager m_descriptorMng{};

	utils::DeletionQueue m_deletionQueue;

	const int MAX_FRAMES_IN_FLIGHT{2};

#ifdef NDEBUG
	const bool m_enableValidationLayers{false};
#else
	const bool m_enableValidationLayers{true};
#endif

	bool m_initialized{false};
	uint32_t m_currentFrame{0};
	bool m_updateFramebuffers{false};

	GUIOverlay *m_gui{nullptr};

#pragma endregion
public:
	Renderer(Window *window) : m_window(window) { on_awake(); }
	Renderer(Window *window, RendererSettings settings) : m_window(window), m_settings(settings) { on_awake(); }
	Renderer(Window *window, RenderPipeline pipeline, RendererSettings settings = {}) : m_window(window), m_settings(settings), m_pipeline(pipeline) { m_frames.resize(MAX_FRAMES_IN_FLIGHT); }

#pragma region Getters & Setters

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
			m_updateFramebuffers = true;
		}
	}
	inline void set_shadow_quality(ShadowResolution quality)
	{
		m_settings.shadowResolution = quality;
		if (m_initialized)
		{
			m_updateFramebuffers = true;
		}
	}
	inline void set_color_format(ColorFormatType color)
	{
		m_settings.colorFormat = color;
		if (m_initialized)
			m_updateFramebuffers = true;
	}
	inline void set_depth_format(DepthFormatType d)
	{
		m_settings.depthFormat = d;
		if (m_initialized)
			m_updateFramebuffers = true;
	}
	inline void set_sync_type(SyncType sync)
	{
		m_settings.screenSync = sync;
		if (m_initialized)
			m_updateFramebuffers = true;
	}

	inline void enable_depth_test(bool op) { m_settings.depthTest = op; }
	inline void enable_depth_writes(bool op) { m_settings.depthWrite = op; }

	inline void enable_gui_overlay(bool op) { m_settings.enableUI; }

	inline void set_gui_overlay(GUIOverlay *gui)
	{
		m_gui = gui;
		static_cast<ForwardPass *>(m_pipeline.renderpasses[1])->set_gui(gui);
	}

	inline GUIOverlay *get_gui_overlay()
	{
		return m_gui;
	}
	inline void set_hardware_depth_bias(bool op) { m_settings.enableHardwareDepthBias = op; };

	inline RenderPipeline get_render_pipeline() const { return m_pipeline; }

#pragma endregion
#pragma region Core Functions

	/**
	 * Inits the renderer.
	 */
	inline void init()
	{
		on_init();
		m_initialized = true;
	}
	/**
	 * Standalone pre-implemented render loop for the renderer.
	 */
	virtual void run(Scene *const scene);
	/**
	 * Renders a given scene.
	 */
	virtual void render(Scene *const scene);
	/**
	 * Shut the renderer down.
	 */
	void shutdown();

protected:
	/*
	What to do when instancing the renderer
	*/
	virtual void on_awake();
	/*
	What to do when initiating the renderer
	*/
	virtual void on_init();
	/*
	What to do just before rendering
	*/
	virtual void on_before_render(Scene *const scene);
	/*
	What to do just before rendering
	*/
	virtual void on_after_render(VkResult &renderResult, Scene *const scene);
	/*
	What to do when shutting down the renderer
	*/
	virtual void on_shutdown();

#pragma endregion
/*
	////////////////////////////////////////////////////////////////////////////////////

	Implementation of this region can be found in the module ==>> vk_renderer_api_mgr.cpp

	////////////////////////////////////////////////////////////////////////////////////
*/
#pragma region Vulkan Management

	/*
	Init renderpasses and create framebuffers and image resources attached to them
	*/
	virtual void init_renderpasses();

	/*
	Render flow control objects creation
	*/
	virtual void init_control_objects();

	/*
	Descriptor pool and layouts creation
	*/
	virtual void init_descriptors();

	/*
	Shader pass creation
	*/
	virtual void init_shaderpasses();

	/*
	Resource like samplers, base textures and misc creation
	*/
	virtual void init_resources();

	/*
	Clean and recreates swapchain and framebuffers in the renderer. Useful to use when resizing or reconfiguring context
	*/
	virtual void update_renderpasses();

	void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

#pragma endregion
/*
	////////////////////////////////////////////////////////////////////////////////////

	Implementation of this region can be found in the module ==>> vk_renderer_data_mgr.cpp

	////////////////////////////////////////////////////////////////////////////////////
*/
#pragma region Data Management
	/*
	Geometry vertex and index buffers upload to GPU
	*/
	virtual void upload_geometry_data(Geometry *const g);
	/*
	Object descriptor layouts uniforms buffer upload to GPU
	*/
	virtual void upload_object_data(Scene *const scene);
	/*
	Global descriptor layouts uniforms buffer upload to GPU
	*/
	virtual void upload_global_data(Scene *const scene);

	/*
	Initialize and setup textures and uniforms in given material
	*/
	virtual void setup_material(Material *const mat);
	/*
	Texture setup and upload to GPU. Mipmap and sampler creation
	*/
	virtual void upload_texture(Texture *const t);
#pragma region GUI
	/*
	Initialize gui layout in case ther's one enabled
	*/
	virtual void init_gui();

#pragma endregion
};

VULKAN_ENGINE_NAMESPACE_END

#endif // VK_RENDERER