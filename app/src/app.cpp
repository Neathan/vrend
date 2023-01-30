#include "app.h"

#include <glad/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include <stdexcept>

#include "data/scene.h"
#include "data/asset_manager.h"


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
	Scene scene;

	AssetManager assetManager(&m_renderer);

	std::shared_ptr<Model> model = assetManager.loadModel(assetManager.loadModelSource("assets/models/gltf/helmet.glb"));
	std::shared_ptr<Model> model2 = assetManager.loadModel(assetManager.loadModelSource("assets/models/gltf/cube_textured.glb"));

	Entity entity1 = scene.createEntity();
	Entity entity2 = scene.createEntity();
	entity1.addComponent<ModelComponent>(model);
	entity2.addComponent<ModelComponent>(model2);

	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		m_renderer.newFrame();

		ViewUniformData *viewData = m_renderer.getCurrentViewUniformBuffer();
		viewData->view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, -5.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f));
		viewData->proj = glm::perspective(
			glm::radians(45.0f),
			1920.0f / 1080.0f,
			0.1f,
			100.0f);
		viewData->proj[1][1] *= -1; // Invert Y clip coordinates (OpenGL artifact)

		m_renderer.prepare();

		entity1.getComponent<TransformComponent>().matrix = glm::rotate(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		entity2.getComponent<TransformComponent>().matrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		scene.render(m_renderer);

		m_renderer.execute();
	}

	// Wait for queue completion, ensures clean shutdown
	m_renderer.waitForIdle();

	scene.destroy();
}

