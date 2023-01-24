#include "model.h"

#include <tiny_gltf.h>

#include <stdexcept>
#include <unordered_map>

#include "log.h"
#include "graphics/memory.h"


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


std::array<VkVertexInputBindingDescription, 3> Model::getBindingDescription() {
	std::array<VkVertexInputBindingDescription, 3>  bindingDescriptions{};

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(float) * 3;
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	bindingDescriptions[1].binding = 1;
	bindingDescriptions[1].stride = sizeof(float) * 2;
	bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	bindingDescriptions[2].binding = 2;
	bindingDescriptions[2].stride = sizeof(float) * 3;
	bindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::array<VkVertexInputAttributeDescription, 3> Model::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = 0;

	attributeDescriptions[1].binding = 1;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = 0;

	attributeDescriptions[2].binding = 2;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = 0;

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
	VkDeviceMemory vertexStagingMemory, modelVertexMemory;

	VkBuffer vertexStagingBuffer = createEmptyBuffer(
		modelSource.getVertexData().size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		renderer.getDevice());

	VkBuffer vertexBuffer = createEmptyBuffer(
		modelSource.getVertexData().size(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		renderer.getDevice());


	VkMemoryRequirements stagingMemoryRequirements;
	vkGetBufferMemoryRequirements(renderer.getDevice().getLogicalDevice(), vertexStagingBuffer, &stagingMemoryRequirements);

	VkMemoryAllocateInfo vertexStagingAllocationInfo{};
	vertexStagingAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vertexStagingAllocationInfo.allocationSize = stagingMemoryRequirements.size;
	vertexStagingAllocationInfo.memoryTypeIndex = findMemoryType(
		stagingMemoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		renderer.getDevice());

	if (vkAllocateMemory(renderer.getDevice().getLogicalDevice(), &vertexStagingAllocationInfo, nullptr, &vertexStagingMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate staging buffer memory");
		throw std::runtime_error("Failed to allocate staging buffer memory");
	}

	vkBindBufferMemory(renderer.getDevice().getLogicalDevice(), vertexStagingBuffer, vertexStagingMemory, 0);


	VkMemoryRequirements vertexMemoryRequirements;
	vkGetBufferMemoryRequirements(renderer.getDevice().getLogicalDevice(), vertexBuffer, &vertexMemoryRequirements);

	VkMemoryAllocateInfo veretxAllocationInfo{};
	veretxAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	veretxAllocationInfo.allocationSize = vertexMemoryRequirements.size;
	veretxAllocationInfo.memoryTypeIndex = findMemoryType(
		vertexMemoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		renderer.getDevice());

	if (vkAllocateMemory(renderer.getDevice().getLogicalDevice(), &veretxAllocationInfo, nullptr, &modelVertexMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate buffer memory");
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	vkBindBufferMemory(renderer.getDevice().getLogicalDevice(), vertexBuffer, modelVertexMemory, 0);


	void *data;
	vkMapMemory(renderer.getDevice().getLogicalDevice(), vertexStagingMemory, 0, modelSource.getVertexData().size(), 0, &data);
	memcpy(data, modelSource.getVertexData().data(), modelSource.getVertexData().size());
	vkUnmapMemory(renderer.getDevice().getLogicalDevice(), vertexStagingMemory);

	VkCommandBuffer commandBuffer = renderer.prepareSingleCommand();
	VkBufferCopy copyRegion{};
	copyRegion.size = modelSource.getVertexData().size();
	vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer, vertexBuffer, 1, &copyRegion);
	renderer.executeSingleCommand(commandBuffer);
	vkDestroyBuffer(renderer.getDevice().getLogicalDevice(), vertexStagingBuffer, nullptr);
	vkFreeMemory(renderer.getDevice().getLogicalDevice(), vertexStagingMemory, nullptr);


	std::vector<VkImage> images;
	std::vector<VkDeviceMemory> imageMemoryList;
	std::vector<VkImageView> imageViews;
	std::vector<VkSampler> imageSamplers;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(renderer.getDevice().getPhysicalDevice(), &properties);

	for (const auto &texture : modelSource.getTextures()) {
		const Image &imageSource = modelSource.getImages().at(texture.image);
		const std::byte *imageData = modelSource.getImageData().data() + imageSource.offset;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView = loadImage(
			(void*)imageData, imageSource.size,
			imageSource.width, imageSource.height,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT,
			renderer, image, imageMemory);


		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = texture.mag;
		samplerInfo.minFilter = texture.min;
		samplerInfo.addressModeU = texture.wrapU;
		samplerInfo.addressModeV = texture.wrapV;
		samplerInfo.addressModeW = texture.wrapW;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		// TODO: Implement below settings (requires changes to ModelSource and converter)
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSampler imageSampler;
		if (vkCreateSampler(renderer.getDevice().getLogicalDevice(), &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
			LOG_ERROR("Failed to create texture sampler");
			throw std::runtime_error("Failed to create texture sampler");
		}

		images.push_back(image);
		imageMemoryList.push_back(imageMemory);
		imageViews.push_back(imageView);
		imageSamplers.push_back(imageSampler);
	}

	return std::make_unique<Model>(
		modelVertexMemory,
		vertexBuffer,
		renderer.getDevice().getLogicalDevice(),
		modelSource.getMeshes(),
		images,
		imageMemoryList,
		imageViews,
		imageSamplers);
}

Model::~Model() {
	for (const auto &imageSampler : m_imageSamplers) {
		vkDestroySampler(m_device, imageSampler, nullptr);
	}

	for (const auto &imageView : m_imageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	for (const auto &image : m_images) {
		vkDestroyImage(m_device, image, nullptr);
	}

	for (const auto &imageMemory : m_imageMemory) {
		vkFreeMemory(m_device, imageMemory, nullptr);
	}

	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);

	vkFreeMemory(m_device, m_modelMemory, nullptr);
}
