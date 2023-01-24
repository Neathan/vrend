#include "renderer.h"

#include <stdexcept>
#include <array>

#include "log.h"
#include "data/model.h"

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		LOG_DEBUG(pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		LOG_INFO(pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		LOG_WARN(pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		LOG_ERROR(pCallbackData->pMessage);
		break;
	}

	return VK_FALSE;
}

static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


Renderer::~Renderer() {
	m_descriptorManager.destroy();

	m_swapChain.destroy();

	m_renderPass.destroy();
	m_pipeline.destory();

	// Destroy sync objects
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(m_device.getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device.getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device.getLogicalDevice(), m_inFlightFences[i], nullptr);
	}

	// Destroy command pool
	vkDestroyCommandPool(m_device.getLogicalDevice(), m_commandPool, nullptr);

	m_device.destroy();

	destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void Renderer::init(const AppInfo& info, GLFWwindow *window) {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = info.name.c_str();
	appInfo.applicationVersion = VK_MAKE_API_VERSION(
		info.versionVariant,
		info.versionMajor,
		info.versionMinor,
		info.versionPatch);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Validation layers
	m_validator.add({
		"VK_LAYER_KHRONOS_validation"
	});
	if (!m_validator.valid()) {
		LOG_ERROR("Unable to install validation layers. Layer collection not supported.");
		throw std::runtime_error("Unable to install validation layers. Layer collection not supported.");
	}

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	createInfo.enabledLayerCount = static_cast<uint32_t>(m_validator.getLayers().size());
	createInfo.ppEnabledLayerNames = m_validator.getLayers().data();

	configureDebugCallback(debugCreateInfo);
	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;

	// Instance extensions
	m_instanceExtensions.addGLFW();
	m_instanceExtensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensions.getExtensions().size());
	createInfo.ppEnabledExtensionNames = m_instanceExtensions.getExtensions().data();

	// Create instance
	if (vkCreateInstance(&createInfo, nullptr, &m_instance)) {
		LOG_ERROR("Failed to create Vulkan instance");
		throw std::runtime_error("Failed to create Vulkan instance");
	}

	// Debug messenger EXT
	if (createDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		LOG_ERROR("Failed to set up debug messenger EXT");
		throw std::runtime_error("Failed to set up debug messenger EXT");
	}

	// Create surface
	if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS) {
		LOG_ERROR("Failed to create window surface");
		throw std::runtime_error("Failed to create window surface");
	}

	// Device extensions
	m_deviceExtensions.add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// Find physical device and create logical
	m_device.init(m_instance, m_surface, m_validator, m_deviceExtensions);

	// Create swap chain
	m_swapChain.init(m_device, m_surface, window);

	// Create render pass
	m_renderPass.init(m_device, m_swapChain.getImageFormat());

	// Create descriptor set layout (could probably move to DescriptorManager)
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(m_device.getLogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
		LOG_ERROR("Failed to create descriptor set layout");
		throw std::runtime_error("Failed to create descriptor set layout");
	}

	// Create graphics pipeline
	m_pipeline.init(m_device, m_renderPass, m_swapChain, "assets/shaders/static.vert.spv", "assets/shaders/static.frag.spv", m_descriptorSetLayout);

	// Create frame buffers
	m_swapChain.createFrameBuffers(m_renderPass);

	// Create command pool
	QueueFamilyIndices queueFamilyIndices = m_device.findQueueFamilies(m_surface);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(m_device.getLogicalDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
		LOG_ERROR("Failed to create command pool");
		throw std::runtime_error("Failed to create command pool");
	}

	// Create command buffers
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

	if (vkAllocateCommandBuffers(m_device.getLogicalDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate command buffers");
		throw std::runtime_error("Failed to allocate command buffers");
	}

	// Create sync objects
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // To resolve first frame issue

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_device.getLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {

			LOG_ERROR("Failed to create semaphores");
			throw std::runtime_error("Failed to create semaphores");
		}
	}

	// Create descriptors
	m_descriptorManager.setLayout(m_descriptorSetLayout);
	m_descriptorManager.init(MAX_FRAMES_IN_FLIGHT, m_device);

	LOG_DEBUG("Renderer initialized");
}

void Renderer::newFrame() {
	// Wait for frame
	vkWaitForFences(m_device.getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	// Acquire next framebuffer image
	VkResult result = vkAcquireNextImageKHR(m_device.getLogicalDevice(), m_swapChain.getSwapChain(), UINT64_MAX,
		m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_imageIndex);

	// Check if swap chain has become incompatible (window resize etc)
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		// TODO:
		// recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		LOG_ERROR("Failed to acquire swap chain image");
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	// Only reset the fence if we are submitting work
	vkResetFences(m_device.getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame]);

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
}

void Renderer::prepare() {
	// Begin command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS) {
		LOG_ERROR("Failed to begin recording command buffer");
		throw std::runtime_error("Failed to begin recording command buffer");
	}

	// Begin render pass
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass.getRenderPass();
	renderPassInfo.framebuffer = m_swapChain.getFramebuffers()[m_imageIndex];

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapChain.getExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_commandBuffers[m_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getPipeline());
	vkCmdBindDescriptorSets(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getLayout(), 0, 1, &m_descriptorManager.getSets()[m_currentFrame], 0, nullptr);
}

void Renderer::execute() {
	// End render pass
	vkCmdEndRenderPass(m_commandBuffers[m_currentFrame]);
	// End command buffer
	if (vkEndCommandBuffer(m_commandBuffers[m_currentFrame]) != VK_SUCCESS) {
		LOG_ERROR("Failed to record command buffer");
		throw std::runtime_error("Failed to record command buffer");
	}

	// Queue buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
		LOG_ERROR("Failed to submit draw command buffer");
		throw std::runtime_error("Failed to submit draw command buffer");
	}

	// Submit result to swap chain
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain.getSwapChain()};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_imageIndex;

	presentInfo.pResults = nullptr; // Optional

	VkResult result = vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);

	// Check if swap chain has become incompatible (window resize etc)
	//  or if swap chain is suboptimal 
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) { // TODO: || m_framebufferResized
		// TODO:
		// m_framebufferResized = false;
		// recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		LOG_ERROR("Failed to present swap chain image");
		throw std::runtime_error("Failed to present swap chain image");
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::waitForIdle() {
	vkDeviceWaitIdle(m_device.getLogicalDevice());
}

void Renderer::addModelCommand(const Model *model) {
	for (const auto &[nodeIndex, meshCollection] : model->getMeshes()) {

		m_descriptorManager.setSamplerDescriptorSet(model->getImageViews()[0], model->getImageSamplers()[0], m_currentFrame);

		for (const auto &mesh : meshCollection) {
			vkCmdBindVertexBuffers(m_commandBuffers[m_currentFrame], 0, 3, model->getVertexBufferAsArray().data(), mesh.getVertexOffsets().data());  // TODO: Offset: Add other primitives (normal, texture coordinate etc)
			vkCmdBindIndexBuffer(m_commandBuffers[m_currentFrame], model->getVertexBuffer(), mesh.getIndexOffset(), VK_INDEX_TYPE_UINT16);

			vkCmdDrawIndexed(m_commandBuffers[m_currentFrame], static_cast<uint32_t>(mesh.getIndexCount()), 1, 0, 0, 0);
		}
	}
}

VkCommandBuffer Renderer::prepareSingleCommand() const {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device.getLogicalDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void Renderer::executeSingleCommand(VkCommandBuffer commandBuffer) const {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device.getGraphicsQueue());

	vkFreeCommandBuffers(m_device.getLogicalDevice(), m_commandPool, 1, &commandBuffer);
}

void Renderer::configureDebugCallback(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}
