#include "descriptor.h"
// 
// #include <array>
// 
// #include "log.h"
// 
// void DescriptorManager::destroy() {
// 	for (int i = 0; i < m_uniformBuffers.size(); ++i) {
// 		m_uniformBuffers[i].destroy();
// 	}
// 
// 	vkDestroyDescriptorPool(m_device, m_pool, nullptr);
// 	vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
// }
// 
// void DescriptorManager::init(int count, const Device &device) {
// 	m_device = device.getLogicalDevice();
// 	for (int i = 0; i < count; ++i) {
// 		m_uniformBuffers.emplace_back(device);
// 	}
// 
// 
// 	VkDescriptorSetLayoutBinding uboLayoutBinding{};
// 	uboLayoutBinding.binding = 0;
// 	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
// 	uboLayoutBinding.descriptorCount = 1;
// 	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
// 
// 	VkDescriptorSetLayoutBinding albedoSamplerLayoutBinding{};
// 	albedoSamplerLayoutBinding.binding = 1;
// 	albedoSamplerLayoutBinding.descriptorCount = 1;
// 	albedoSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	albedoSamplerLayoutBinding.pImmutableSamplers = nullptr;
// 	albedoSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
// 
// 	VkDescriptorSetLayoutBinding normalSamplerLayoutBinding{};
// 	normalSamplerLayoutBinding.binding = 1;
// 	normalSamplerLayoutBinding.descriptorCount = 1;
// 	normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
// 	normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
// 
// 
// 	std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings = { uboLayoutBinding, albedoSamplerLayoutBinding, normalSamplerLayoutBinding };
// 	VkDescriptorSetLayoutCreateInfo layoutInfo{};
// 	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
// 	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
// 	layoutInfo.pBindings = layoutBindings.data();
// 
// 	if (vkCreateDescriptorSetLayout(device.getLogicalDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
// 		LOG_ERROR("Failed to create descriptor set layout");
// 		throw std::runtime_error("Failed to create descriptor set layout");
// 	}
// 
// 
// 	std::array<VkDescriptorPoolSize, 2> poolSizes = {
// 		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(count) },
// 		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(count * SCENE_MAX_OBJECTS) }
// 	};
// 
// 	VkDescriptorPoolCreateInfo poolInfo{};
// 	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
// 	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
// 	poolInfo.pPoolSizes = poolSizes.data();
// 	poolInfo.maxSets = static_cast<uint32_t>(count);
// 	poolInfo.flags = 0;
// 
// 	if (vkCreateDescriptorPool(device.getLogicalDevice(), &poolInfo, nullptr, &m_pool) != VK_SUCCESS) {
// 		LOG_ERROR("Failed to create descriptor pool");
// 		throw std::runtime_error("Failed to create descriptor pool");
// 	}
// 
// 
// 	std::vector<VkDescriptorSetLayout> layouts(count, m_layout);
// 	VkDescriptorSetAllocateInfo allocInfo{};
// 	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
// 	allocInfo.descriptorPool = m_pool;
// 	allocInfo.descriptorSetCount = static_cast<uint32_t>(count);
// 	allocInfo.pSetLayouts = layouts.data();
// 
// 	m_sets.resize(count);
// 	if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, m_sets.data()) != VK_SUCCESS) {
// 		LOG_ERROR("Failed to allocate descriptor sets");
// 		throw std::runtime_error("Failed to allocate descriptor sets");
// 	}
// 
// 	for (size_t i = 0; i < count; ++i) {
// 		VkDescriptorBufferInfo bufferInfo{};
// 		bufferInfo.buffer = m_uniformBuffers[i].getBuffer();
// 		bufferInfo.offset = 0;
// 		bufferInfo.range = m_uniformBuffers[i].getSize();
// 
// 		VkWriteDescriptorSet descriptorWrite{};
// 		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 		descriptorWrite.dstSet = m_sets[i];
// 		descriptorWrite.dstBinding = 0;
// 		descriptorWrite.dstArrayElement = 0;
// 
// 		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
// 		descriptorWrite.descriptorCount = 1;
// 
// 		descriptorWrite.pBufferInfo = &bufferInfo;
// 
// 		vkUpdateDescriptorSets(device.getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
// 	}
// }
// 
// VkWriteDescriptorSet createTextureWriteInfo(uint32_t binding, VkDescriptorSet set, const Texture& texture) {
// 	VkDescriptorImageInfo imageInfo{};
// 	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	imageInfo.imageView = texture.view;
// 	imageInfo.sampler = texture.sampler;
// 
// 	VkWriteDescriptorSet descriptorWrite{};
// 	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 	descriptorWrite.dstSet = set;
// 	descriptorWrite.dstBinding = binding;
// 	descriptorWrite.dstArrayElement = 0;
// 
// 	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	descriptorWrite.descriptorCount = 1;
// 
// 	descriptorWrite.pImageInfo = &imageInfo;
// 
// 	return descriptorWrite;
// }
// 
// void DescriptorManager::bindMaterialDescriptorOffsets(std::vector<Material> &materials, int frame) {
// 	std::vector<VkWriteDescriptorSet> writeInfos;
// 	for (size_t i = 0; i < materials.size(); ++i) {
// 		materials[i].offset = i;
// 		writeInfos.push_back(createTextureWriteInfo(0, m_sets[frame], materials[i].albedo));
// 		writeInfos.push_back(createTextureWriteInfo(1, m_sets[frame], materials[i].normal));
// 
// 	}
// 	vkCmdBindDescriptorSets()
// 	vkUpdateDescriptorSets(m_device, 2, writeInfos.data(), 0, nullptr);
// }
// 
// // void DescriptorManager::setSamplerDescriptorSet(VkImageView imageView, VkSampler imageSampler, int frame) {
// // 	VkDescriptorImageInfo imageInfo{};
// // 	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// // 	imageInfo.imageView = imageView;
// // 	imageInfo.sampler = imageSampler;
// // 
// // 	VkWriteDescriptorSet descriptorWrite{};
// // 	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// // 	descriptorWrite.dstSet = m_sets[frame];
// // 	descriptorWrite.dstBinding = 1;
// // 	descriptorWrite.dstArrayElement = 0;
// // 
// // 	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// // 	descriptorWrite.descriptorCount = 1;
// // 
// // 	descriptorWrite.pImageInfo = &imageInfo;
// // 
// // 	vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
// // }
