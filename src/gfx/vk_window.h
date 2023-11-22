#pragma once
#include "backend/vk_core.h"
#include "backend/vk_utils.h"

namespace vkeng
{
	/**
	 * Class containing all functionality related to a window. It uses GLFW as context creator.
	 */
	class Window
	{
	private:
		GLFWwindow *m_window{nullptr};
		std::string m_title{};
		int m_width;
		int m_height;
		VkExtent2D *m_extent;
		VkSurfaceKHR *m_surface;

		bool m_resized{false};
		bool m_resizeable;
		bool m_fullscreen;
		glm::ivec2 m_screenPos = glm::ivec2(45, 45);

	public:
		Window(const std::string t, uint32_t w, uint32_t h, bool resizable = true, bool fullscreen = false) : m_title(t), m_width(w), m_height(h), m_extent(new VkExtent2D{}), m_surface(new VkSurfaceKHR{}), m_resizeable{resizable}, m_fullscreen{fullscreen} {}
		~Window()
		{
			delete m_extent;
			delete m_surface;
		}
		void init();
		inline void destroy() { glfwDestroyWindow(m_window); }

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

				glfwSetWindowMonitor(m_window, NULL, m_screenPos.x, m_screenPos.y, m_width, m_height, GLFW_DONT_CARE);
			}
			else
			{
				const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			}
		}
		inline bool is_resizable() { return m_resizeable; }
		inline void set_resizeable(bool t) { m_resizeable = t; /*glfwsetwindowresize(GLFW_RESIZABLE, t);*/ }
		inline glm::ivec2 get_position() { return m_screenPos; }
		inline void set_position(glm::ivec2 p)
		{
			m_screenPos = p;
			glfwSetWindowPos(m_window, p.x, p.y);
		}
		inline uint32_t get_width() { return m_width; }
		inline uint32_t get_height() { return m_height; }
		inline GLFWwindow *const get_window_obj() const { return m_window; }
		inline VkExtent2D *const get_extent() const { return m_extent; }
		inline VkSurfaceKHR *const get_surface() const { return m_surface; }
		inline void poll_events() { glfwPollEvents(); }
		
		void set_keyboard_callback(std::function<void()>&& function);
		void set_mouse_callback(std::function<void()>&& function);
		void set_window_callback(std::function<void()>&& function);
	};
}