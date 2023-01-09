#include "renderer.h"

#include <stdexcept>

#include "log.h"


VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData) {

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

static VkResult createDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugUtilsMessengerEXT *pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void destroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks *pAllocator) {

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

Renderer::~Renderer() {
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

	LOG_DEBUG("Renderer initialized");
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
