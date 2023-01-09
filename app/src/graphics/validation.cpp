#include "validation.h"

#include <glad/vulkan.h>

#include <cstdint>
#include <cstring>

#include "log.h"


bool Validator::valid() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName : m_layers) {
		bool layerFound = false;
		
		for (const VkLayerProperties &layer : availableLayers) {
			if (std::strcmp(layer.layerName, layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			LOG_WARN("Validation layer not available: {}", layerName);
			return false;
		}
	}
	return true;
}
