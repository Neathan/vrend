#include "extensions.h"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>
#include <set>


void Extensions::addGLFW() {
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	m_extensions.insert(m_extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);
}

bool Extensions::deviceCompatible(VkPhysicalDevice device) const {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(m_extensions.begin(), m_extensions.end());
	for (const auto &extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}
