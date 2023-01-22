#include "model.h"

#include <tiny_gltf.h>

#include <stdexcept>
#include <unordered_map>

#include "log.h"


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const Device &device) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device.getPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOG_ERROR("Failed to find suitable memory type");
	throw std::runtime_error("Failed to find suitable memory type");
}


// void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const Device &device, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
// 	VkBufferCreateInfo bufferInfo{};
// 	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
// 	bufferInfo.size = size;
// 
// 	bufferInfo.usage = usage;
// 	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 	bufferInfo.flags = 0; // Optional (spares buffer memory)
// 
// 	if (vkCreateBuffer(device.getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
// 		LOG_ERROR("Failed to create buffer");
// 		throw std::runtime_error("Failed to create buffer");
// 	}
// 
// 	VkMemoryRequirements memRequirements;
// 	vkGetBufferMemoryRequirements(device.getLogicalDevice(), buffer, &memRequirements);
// 
// 	VkMemoryAllocateInfo allocInfo{};
// 	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
// 	allocInfo.allocationSize = memRequirements.size;
// 	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, device);
// 
// 	if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
// 		LOG_ERROR("Failed to allocate buffer memory");
// 		throw std::runtime_error("Failed to allocate buffer memory");
// 	}
// 
// 	vkBindBufferMemory(device.getLogicalDevice(), buffer, bufferMemory, 0);
// }
// 
// struct Buffer {
// 	VkBuffer buffer;
// 	VkDeviceMemory memory;
// };
// 
// template<typename T>
// Buffer loadBuffer(T *data, size_t size, const Renderer &renderer, VkBufferUsageFlags usageFlags) {
// 	VkDeviceSize bufferSize = sizeof(data[0]) * size;
// 
// 	VkBuffer stagingBuffer;
// 	VkDeviceMemory stagingBufferMemory;
// 	createBuffer(
// 		bufferSize,
// 		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
// 		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
// 		renderer.getDevice(),
// 		stagingBuffer,
// 		stagingBufferMemory);
// 
// 	void *bufferData;
// 	vkMapMemory(renderer.getDevice().getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &bufferData);
// 	memcpy(bufferData, data, (size_t)bufferSize);
// 	vkUnmapMemory(renderer.getDevice().getLogicalDevice(), stagingBufferMemory);
// 
// 	VkBuffer buffer;
// 	VkDeviceMemory bufferMemory;
// 	createBuffer(bufferSize,
// 		VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags,
// 		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
// 		renderer.getDevice(),
// 		buffer,
// 		bufferMemory);
// 
// 	// Copy data from staging buffer to target buffer
// 	VkCommandBuffer commandBuffer = renderer.prepareSingleCommand();
// 	VkBufferCopy copyRegion{};
// 	copyRegion.size = size;
// 	vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &copyRegion);
// 	renderer.executeSingleCommand(commandBuffer);
// 
// 	vkDestroyBuffer(renderer.getDevice().getLogicalDevice(), stagingBuffer, nullptr);
// 	vkFreeMemory(renderer.getDevice().getLogicalDevice(), stagingBufferMemory, nullptr);
// 
// 	return { buffer, bufferMemory };
// }


std::array<VkVertexInputBindingDescription, 1> Model::getBindingDescription() {
	std::array<VkVertexInputBindingDescription, 1>  bindingDescriptions{};

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(float) * 3;
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::array<VkVertexInputAttributeDescription, 1> Model::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = 0;

	return attributeDescriptions;
}


VkBuffer createEmptyBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const Device &device) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;

	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.flags = 0; // Optional (spares buffer memory)

	VkBuffer buffer;

	if (vkCreateBuffer(device.getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		LOG_ERROR("Failed to create buffer");
		throw std::runtime_error("Failed to create buffer");
	}

	return buffer;
}


std::unique_ptr<Model> Model::load(const ModelSource &modelSource, const Renderer &renderer) {
	const std::vector<std::byte> &vertexData = modelSource.getVertexData();

	VkDeviceSize modelByteSize = vertexData.size(); // TODO: Add images

	VkDeviceMemory stagingMemory, modelMemory;

	VkBuffer stagingBuffer = createEmptyBuffer(
		vertexData.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		renderer.getDevice());

	VkBuffer buffer = createEmptyBuffer(
		vertexData.size(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		renderer.getDevice());


	VkMemoryRequirements stagingMemoryRequirements;
	vkGetBufferMemoryRequirements(renderer.getDevice().getLogicalDevice(), stagingBuffer, &stagingMemoryRequirements);

	// TODO: Concatenate requirements of vertex and image

	VkMemoryAllocateInfo stagingAllocationInfo{};
	stagingAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	stagingAllocationInfo.allocationSize = stagingMemoryRequirements.size;
	stagingAllocationInfo.memoryTypeIndex = findMemoryType(
		stagingMemoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		renderer.getDevice());

	if (vkAllocateMemory(renderer.getDevice().getLogicalDevice(), &stagingAllocationInfo, nullptr, &stagingMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate staging buffer memory");
		throw std::runtime_error("Failed to allocate staging buffer memory");
	}

	vkBindBufferMemory(renderer.getDevice().getLogicalDevice(), stagingBuffer, stagingMemory, 0);  // Note: Offset for image should be vertexData.size()


	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(renderer.getDevice().getLogicalDevice(), buffer, &memoryRequirements);

	// TODO: Concatenate requirements of vertex and image

	VkMemoryAllocateInfo allocationInfo{};
	allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocationInfo.allocationSize = memoryRequirements.size;
	allocationInfo.memoryTypeIndex = findMemoryType(
		memoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		renderer.getDevice());

	if (vkAllocateMemory(renderer.getDevice().getLogicalDevice(), &allocationInfo, nullptr, &modelMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate buffer memory");
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	vkBindBufferMemory(renderer.getDevice().getLogicalDevice(), buffer, modelMemory, 0);  // Note: Offset for image should be vertexData.size()


	void *data;
	vkMapMemory(renderer.getDevice().getLogicalDevice(), stagingMemory, 0, vertexData.size(), 0, &data);  // Note: Offset for image should be vertexData.size()
	memcpy(data, vertexData.data(), vertexData.size());
	vkUnmapMemory(renderer.getDevice().getLogicalDevice(), stagingMemory);

	// TODO: Image mapping


	VkCommandBuffer commandBuffer = renderer.prepareSingleCommand();
	VkBufferCopy copyRegion{};
	copyRegion.size = vertexData.size();
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &copyRegion);
	renderer.executeSingleCommand(commandBuffer);
	vkDestroyBuffer(renderer.getDevice().getLogicalDevice(), stagingBuffer, nullptr);

	// TODO: Image copy


	vkFreeMemory(renderer.getDevice().getLogicalDevice(), stagingMemory, nullptr);

	return std::make_unique<Model>(modelMemory, buffer, renderer.getDevice().getLogicalDevice(), modelSource.getMeshes());
}

Model::~Model() {
	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);

	vkFreeMemory(m_device, m_modelMemory, nullptr);
}
