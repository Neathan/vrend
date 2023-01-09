#pragma once

#include <glad/vulkan.h>

#include <optional>

#include "validation.h"
#include "extensions.h"


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class Device {
public:
	~Device();

	void init(VkInstance instance, VkSurfaceKHR surface, const Validator &validator, const Extensions &extensions);

private:
	VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const Extensions &extensions);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	
	VkPhysicalDevice m_physicalDevice{};
	VkDevice m_device{};

	VkQueue m_graphicsQueue{};
	VkQueue m_presentQueue{};

};
