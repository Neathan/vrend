#pragma once

#include <glad/vulkan.h>
#include <glm/glm.hpp>

#include "graphics/device.h"


struct UniformData {
	glm::mat4 view;
	glm::mat4 proj;
};

class UniformBuffer {
public:
	UniformBuffer(const Device& device);

	void destroy();

	VkBuffer getBuffer() const { return m_buffer; }
	VkDeviceMemory getMemory() const { return m_memory; }
	size_t getSize() const { return sizeof(UniformData); }

	UniformData *getData() { return m_data; }

private:
	VkBuffer m_buffer;
	VkDeviceMemory m_memory;

	UniformData* m_data;

	VkDevice m_device;
};
