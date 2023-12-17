#ifndef VK_WINDOW
#define VK_WINDOW

#include "../private/vk_core.h"
#include "../private/vk_utils.h"

namespace vke
{
	/**
	 * Class containing all functionality related to a window. It uses GLFW as context creator.
	 */
	class Window
	{
	private:
		GLFWwindow *m_GLFWwindow{nullptr};
		std::string m_title{};
		int m_width;
		int m_height;
		VkExtent2D *m_extent;
		VkSurfaceKHR *m_surface;

		bool m_resized{false};
		bool m_resizeable;
		bool m_fullscreen;
		glm::ivec2 m_screenPos = glm::ivec2(45, 45);

		// Callbacks
		std::function<void(int, int, int, int)> m_keyCallback;
		std::function<void(int, int)> m_windowSizeCallback;
		std::function<void(double, double)> m_mouseCallBack;

		friend class Renderer;

	public:
		Window(const std::string t, uint32_t w, uint32_t h, bool resizable = true, bool fullscreen = false) : m_title(t), m_width(w), m_height(h), m_extent(new VkExtent2D{}), m_surface(new VkSurfaceKHR{}), m_resizeable{resizable}, m_fullscreen{fullscreen} {}
		~Window()
		{
			delete m_extent;
			delete m_surface;
		}
		void init();
		inline void destroy() { glfwDestroyWindow(m_GLFWwindow); }

		inline void set_size(int w, int h)
		{
			m_width = w;
			m_height = h;
			m_resized = true;
		}
		inline bool is_resized() { return m_resized; }
		inline void set_resized(bool r) { m_resized = r; }
		inline bool is_fullscreen() { return m_fullscreen; }
		inline void set_fullscreen(bool t)
		{
			m_fullscreen = t;
			if (!m_fullscreen)
			{
				glfwSetWindowMonitor(m_GLFWwindow, NULL, m_screenPos.x, m_screenPos.y, m_width, m_height, GLFW_DONT_CARE);
			}
			else
			{
				const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(m_GLFWwindow, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			}
		}
		inline bool is_resizable() { return m_resizeable; }
		inline void set_resizeable(bool t) { m_resizeable = t; /*glfwsetwindowresize(GLFW_RESIZABLE, t);*/ }
		inline glm::ivec2 get_position() { return m_screenPos; }
		inline void set_position(glm::ivec2 p)
		{
			m_screenPos = p;
			glfwSetWindowPos(m_GLFWwindow, p.x, p.y);
		}
		inline uint32_t get_width() { return m_width; }
		inline uint32_t get_height() { return m_height; }
		inline GLFWwindow *const get_window_obj() const { return m_GLFWwindow; }
		inline VkExtent2D *const get_extent() const { return m_extent; }
		inline VkSurfaceKHR *const get_surface() const { return m_surface; }
		inline int get_window_should_close() { return glfwWindowShouldClose(m_GLFWwindow); }
		inline void set_window_should_close(bool op) { glfwSetWindowShouldClose(m_GLFWwindow, op); }
		inline void set_title(const char *title)
		{
			m_title = title;
			glfwSetWindowTitle(m_GLFWwindow, title);
		}
		inline std::string get_title() { return m_title; }
		inline static void poll_events() { glfwPollEvents(); }
		inline static double get_time_elapsed() { return glfwGetTime(); }

		inline void set_key_callback(std::function<void(int, int, int, int)> callback)
		{
			m_keyCallback = callback;
		}

		inline void set_window_size_callback(std::function<void(int, int)> callback)
		{
			m_windowSizeCallback = callback;
		}

		inline void set_mouse_callback(std::function<void(double, double)> callback)
		{
			m_mouseCallBack = callback;
		}
	};
}
#endif