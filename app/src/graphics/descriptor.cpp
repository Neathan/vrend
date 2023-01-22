#include "descriptor.h"

#include "log.h"

void DescriptorManager::destroy() {
	for (int i = 0; i < m_uniformBuffers.size(); ++i) {
		m_uniformBuffers[i].destroy();
	}

	vkDestroyDescriptorPool(m_device, m_pool, nullptr);
	vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
}

void DescriptorManager::init(int count, const Device &device) {
	m_device = device.getLogicalDevice();

	for (int i = 0; i < count; ++i) {
		m_uniformBuffers.emplace_back(device);
	}

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(count);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(count);
	poolInfo.flags = 0;

	if (vkCreateDescriptorPool(device.getLogicalDevice(), &poolInfo, nullptr, &m_pool) != VK_SUCCESS) {
		LOG_ERROR("Failed to create descriptor pool");
		throw std::runtime_error("Failed to create descriptor pool");
	}


	std::vector<VkDescriptorSetLayout> layouts(count, m_layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(count);
	allocInfo.pSetLayouts = layouts.data();

	m_sets.resize(count);
	if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, m_sets.data()) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate descriptor sets");
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	for (size_t i = 0; i < count; ++i) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i].getBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_uniformBuffers[i].getSize();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_sets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(device.getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}
