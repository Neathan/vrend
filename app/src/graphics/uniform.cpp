#include "uniform.h"

#include <stdexcept>

#include "log.h"
#include "graphics/memory.h"


UniformBuffer::UniformBuffer(const Device &device) : m_device(device.getLogicalDevice()) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = getSize();
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device.getLogicalDevice(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS) {
		LOG_ERROR("Failed to create uniform buffer");
		throw std::runtime_error("Failed to create uniform buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.getLogicalDevice(), m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, device);

	if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &m_memory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate uniform buffer memory");
		throw std::runtime_error("Failed to allocate uniform buffer memory");
	}

	vkBindBufferMemory(device.getLogicalDevice(), m_buffer, m_memory, 0);

	vkMapMemory(device.getLogicalDevice(), m_memory, 0, getSize(), 0, (void**)&m_data);
}

void UniformBuffer::destroy() {
	vkDestroyBuffer(m_device, m_buffer, nullptr);
	vkFreeMemory(m_device, m_memory, nullptr);
}

