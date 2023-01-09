#include "app.h"

#include <glad/vulkan.h>

#include <stdexcept>


App::~App() {
	if (!m_window) {
		return;
	}

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void App::init() {
	if (!glfwInit()) {
		throw std::runtime_error("Unable to initialize GLFW");
	}

	int vkVersion = gladLoaderLoadVulkan(nullptr, nullptr, nullptr);
	if (!vkVersion) {
		throw std::runtime_error("Unable to load Vulkan symbols");
	}

	// Create GLFW window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(m_width, m_height, "Vulkan renderer", nullptr, nullptr);

	glfwSetWindowUserPointer(m_window, this);

	m_renderer.init(AppInfo{
		"Vulkan renderer",
		1, 0, 0, 0
	}, m_window);
}

void App::start() {
	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
	}
}

