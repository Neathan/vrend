#include "device.h"

#include <vector>
#include <set>
#include <stdexcept>

#include "log.h"



void Device::destroy() {
	if (m_device) {
		vkDestroyDevice(m_device, nullptr);
	}
}

void Device::init(VkInstance instance, VkSurfaceKHR surface, const Validator& validator, const Extensions& deviceExtensions) {
	// Find physical device
	m_physicalDevice = selectPhysicalDevice(instance, surface, deviceExtensions);

	if (!m_physicalDevice) {
		LOG_ERROR("Unable to find suitable physical device");
		throw std::runtime_error("Unable to find suitable physical device");
	}

	int vkVersion = gladLoaderLoadVulkan(instance, m_physicalDevice, nullptr);
	if (!vkVersion) {
		LOG_ERROR("Unable to load Vulkan symbols for physical device");
		throw std::runtime_error("Unable to load Vulkan symbols for physical device");
	}


	// Create logical device
	QueueFamilyIndices indices = findQueueFamilies(surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Create device info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	// Features
	VkPhysicalDeviceFeatures deviceFeatures{};
	createInfo.pEnabledFeatures = &deviceFeatures;

	// We force required anisotropy to be required
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// Extensions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.getExtensions().size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.getExtensions().data();

	// Validation layers
	createInfo.enabledLayerCount = 0;// static_cast<uint32_t>(validator.getLayers().size());
	createInfo.ppEnabledLayerNames = validator.getLayers().data();

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
		LOG_ERROR("Failed to create logical device");
		//throw std::runtime_error("Failed to create logical device");
	}

	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);


	vkVersion = gladLoaderLoadVulkan(instance, m_physicalDevice, m_device);
	if (!vkVersion) {
		LOG_ERROR("Unable to load Vulkan symbols for logical device");
		throw std::runtime_error("Unable to load Vulkan symbols for logical device");
	}
}

// TODO: Very basic for now, needs to check additional criteria for proper automatic physical device selection
VkPhysicalDevice Device::selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const Extensions &deviceExtensions) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto &device : devices) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Application requires a graphics capable queue family
		QueueFamilyIndices indices = findQueueFamilies(device, surface);

		bool extensionsSupported = deviceExtensions.deviceCompatible(device);

		if (!indices.isComplete() || !extensionsSupported) {
			continue;
		}
		// Application can't function without geometry shaders
		else if (!deviceFeatures.geometryShader) {
			continue;
		}
		else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return device;
		}
	}
	return VK_NULL_HANDLE;
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
	QueueFamilyIndices indices;

	// Assign index to queue families that could be found
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto &queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
		++i;
	}

	return indices;
}
