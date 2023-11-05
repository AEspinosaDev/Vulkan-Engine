#pragma once
#include "vk_core.h"
#include "vk_utils.h"

namespace vkeng {

	class Window
	{
	private:
		GLFWwindow* m_window{ nullptr };
		std::string m_title{};
		uint32_t m_width;
		uint32_t m_height;
		VkExtent2D* m_windowExtent;
	/*	VkSurfaceKHR* m_surface;
		vkutils::EventDispatcher keyDispatcher;*/

	public:

		Window(const std::string t, uint32_t w, uint32_t h) : m_title(t), m_width(w), m_height(h), m_windowExtent(new VkExtent2D{}) {}
		void init();
		void create_surface(VkInstance ins);

		inline GLFWwindow* get_window_obj() { return m_window; }
		inline uint32_t get_width() { return m_width; }
		inline uint32_t get_height() { return m_height; }
		inline VkExtent2D* get_extent() { return m_windowExtent; }
		inline void set_extent(VkExtent2D* e) { m_windowExtent = e; }

		

	};
}