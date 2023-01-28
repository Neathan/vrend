#pragma once

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <memory>

#include "appinfo.h"
#include "graphics/device.h"
#include "graphics/swap_chain.h"
#include "graphics/render_pass.h"
#include "graphics/pipeline.h"
#include "graphics/validation.h"
#include "graphics/extensions.h"
#include "graphics/uniform.h"
#include "graphics/descriptors.h"
#include "graphics/material.h"


class Model;

class Renderer {
public:
	~Renderer();

	void init(const AppInfo &info, GLFWwindow *window);
	void newFrame();
	void prepare();
	void execute();
	void waitForIdle();

	void addTransformCommand(const glm::mat4 &matrix);
	void addModelCommand(const Model *model);
	void updateMaterialSets(Material &material);
	UniformData *getCurrentUniformBuffer() { return m_uniformBuffers[m_currentFrame].getData(); }

	VkCommandBuffer prepareSingleCommand() const;
	void executeSingleCommand(VkCommandBuffer commandBuffer) const;

	VkInstance getInstance() const { return m_instance; }
	const Device &getDevice() const { return m_device; }
	const RenderPass &getRenderPass() const { return m_renderPass; }

	VkCommandBuffer getCurrentCommandBuffer() const { return m_commandBuffers[m_currentFrame]; }

private:
	void configureDebugCallback(VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfo);

	VkInstance m_instance{};
	VkSurfaceKHR m_surface{};

	Device m_device{};
	SwapChain m_swapChain{};
	RenderPass m_renderPass{};
	Pipeline m_pipeline{};

	Validator m_validator{};
	Extensions m_instanceExtensions{};
	Extensions m_deviceExtensions{};

	VkDebugUtilsMessengerEXT m_debugMessenger{};
	
	// Commands
	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	// Sync objects
	std::vector <VkSemaphore> m_imageAvailableSemaphores;
	std::vector <VkSemaphore> m_renderFinishedSemaphores;
	std::vector <VkFence> m_inFlightFences;
	uint32_t m_currentFrame = 0;

	// Descriptors
// 	DescriptorManager m_descriptorManager;
	std::vector<UniformBuffer> m_uniformBuffers;
	std::vector<VkDescriptorSet> m_uniformSets;

	DescriptorLayoutCache m_descriptorLayoutCache;
	DescriptorAllocator m_descriptorAllocator;

	// Render state variables
	uint32_t m_imageIndex;

	static const int MAX_FRAMES_IN_FLIGHT = 2;
};
