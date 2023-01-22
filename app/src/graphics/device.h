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
	void init(VkInstance instance, VkSurfaceKHR surface, const Validator &validator, const Extensions &extensions);

	void destroy();

	QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const {
		return findQueueFamilies(m_physicalDevice, surface);
	};

	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
	VkDevice getLogicalDevice() const { return m_device; }

	VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	VkQueue getPresentQueue() const { return m_presentQueue; }

	explicit operator bool() const noexcept { return m_physicalDevice && m_device && m_graphicsQueue && m_presentQueue; }

private:
	VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const Extensions &extensions);
	
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;

	VkPhysicalDevice m_physicalDevice{};
	VkDevice m_device{};

	VkQueue m_graphicsQueue{};
	VkQueue m_presentQueue{};

};
