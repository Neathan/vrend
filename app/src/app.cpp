#include "app.h"

#include <glad/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

#include "data/model.h"
#include "tools/convert_model.h"


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
	auto modelSource = convertToModelSource("assets/models/gltf/helmet.glb");
	auto model = Model::load(*modelSource.get(), m_renderer);

	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		m_renderer.newFrame();
		m_renderer.prepare();

		UniformData* ubo = m_renderer.getCurrentUniformBuffer();
		ubo->model = glm::rotate(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
// 		ubo->model = glm::mat4(1.0f);
		ubo->view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, -5.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f));
		ubo->proj = glm::perspective(
			glm::radians(45.0f),
			1920.0f / 1080.0f,
			0.1f,
			100.0f);
		ubo->proj[1][1] *= -1; // Invert Y clip coordinates (OpenGL artifact)

		m_renderer.addModelCommand(model.get());

		m_renderer.execute();
	}

	// Wait for queue completion, ensures clean shutdown
	m_renderer.waitForIdle();
}

