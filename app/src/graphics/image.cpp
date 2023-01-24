#include "image.h"

#include "log.h"
#include "graphics/memory.h"

VkMemoryRequirements createImage(uint32_t width, uint32_t height, VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	const Device& device, VkImage &image, VkDeviceMemory &imageMemory) {

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		LOG_ERROR("Failed to create image");
		throw std::runtime_error("Failed to create image");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device.getLogicalDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, device);

	if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate image memory");
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(device.getLogicalDevice(), image, imageMemory, 0);

	return memRequirements;
}

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout sourceLayout,
	VkImageLayout targetLayout, const Renderer &renderer) {

	VkCommandBuffer commandBuffer = renderer.prepareSingleCommand();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = sourceLayout;
	barrier.newLayout = targetLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (sourceLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (sourceLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	renderer.executeSingleCommand(commandBuffer);
}

VkImageView loadImage(void *imageData, size_t imageSize, uint32_t width, uint32_t height,
	VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, const Renderer &renderer,
	VkImage &image, VkDeviceMemory &imageMemory) {

	const Device &device = renderer.getDevice();

	VkMemoryRequirements requirements = createImage(
		width, height, format, tiling,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		device, image, imageMemory);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = imageSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device.getLogicalDevice(), &stagingBufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
		LOG_ERROR("Failed to create staging buffer for image loading");
		throw std::runtime_error("Failed to create staging buffer during image loading");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.getLogicalDevice(), stagingBuffer, &memRequirements);

	VkMemoryAllocateInfo stagingAllocationInfo{};
	stagingAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	stagingAllocationInfo.allocationSize = memRequirements.size;
	stagingAllocationInfo.memoryTypeIndex = findMemoryType(
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, device);

	if (vkAllocateMemory(device.getLogicalDevice(), &stagingAllocationInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate staging buffer memory during image loading");
		throw std::runtime_error("Failed to allocate staging buffer memory during image loading");
	}

	vkBindBufferMemory(device.getLogicalDevice(), stagingBuffer, stagingBufferMemory, 0);

	void *data;
	vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, imageData, imageSize);
	vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);


	transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, renderer);

	VkCommandBuffer commandBuffer = renderer.prepareSingleCommand();
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	renderer.executeSingleCommand(commandBuffer);

	transitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, renderer);

	vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);


	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}


	return imageView;
}
