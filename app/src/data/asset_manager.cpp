#include "asset_manager.h"

#include <stb_image.h>

#include "graphics/renderer.h"
#include "tools/convert_model.h"
#include "log.h"


void transitionImageLayout(VkImage image, VkFormat format,
	VkImageLayout sourceLayout, VkImageLayout targetLayout,
	const Renderer &renderer) {

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

VkBuffer createEmptyBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkDevice device) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;

	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.flags = 0; // Optional (spares buffer memory)

	VkBuffer buffer;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		LOG_ERROR("Failed to create buffer");
		throw std::runtime_error("Failed to create buffer");
	}

	return buffer;
}


AssetManager::AssetManager(Renderer *renderer) : m_renderer(renderer) {
	// Load default textures
	m_defaultMetallicRoughnessImage = loadImage("assets/textures/defaultMetallicRoughness.png", ImageFormat::LINEAR);
	m_defaultNormalImage = loadImage("assets/textures/defaultNormal.png", ImageFormat::LINEAR);
	m_defaultOcclusionImage = loadImage("assets/textures/defaultOcclusion.png", ImageFormat::LINEAR);
	m_defaultEmissiveImage = loadImage("assets/textures/defaultEmissive.png", ImageFormat::SRGB);

	m_defaultMetallicRoughnessTexture = loadTexture(m_defaultMetallicRoughnessImage, TextureProperties{});
	m_defaultNormalTexture = loadTexture(m_defaultNormalImage, TextureProperties{});
	m_defaultOcclusionTexture = loadTexture(m_defaultOcclusionImage, TextureProperties{});
	m_defaultEmissiveTexture = loadTexture(m_defaultEmissiveImage, TextureProperties{});

	m_defaultMetallicRoughnessTexture.setDefault(true);
	m_defaultNormalTexture.setDefault(true);
	m_defaultOcclusionTexture.setDefault(true);
	m_defaultEmissiveTexture.setDefault(true);
}

void AssetManager::destroy() {
	m_defaultMetallicRoughnessTexture.destroy(m_renderer->getDevice().getLogicalDevice());
	m_defaultNormalTexture.destroy(m_renderer->getDevice().getLogicalDevice());
	m_defaultOcclusionTexture.destroy(m_renderer->getDevice().getLogicalDevice());
	m_defaultEmissiveTexture.destroy(m_renderer->getDevice().getLogicalDevice());

	m_defaultMetallicRoughnessImage.destroy(m_renderer->getDevice().getLogicalDevice());
	m_defaultNormalImage.destroy(m_renderer->getDevice().getLogicalDevice());
	m_defaultOcclusionImage.destroy(m_renderer->getDevice().getLogicalDevice());
	m_defaultEmissiveImage.destroy(m_renderer->getDevice().getLogicalDevice());
}

Image AssetManager::loadImage(const std::string &path, ImageFormat format) {
	int width, height, comp;
	stbi_uc *data = stbi_load(path.c_str(), &width, &height, &comp, 4);

	Image image = loadImage(data, width * height * 4 * sizeof(stbi_uc), static_cast<unsigned int>(width), static_cast<unsigned int>(height), format);

	stbi_image_free(data);
	return image;
}

Image AssetManager::loadImage(void *data, size_t size, unsigned int width, unsigned int height, ImageFormat format) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = static_cast<VkFormat>(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage image;
	if (vkCreateImage(m_renderer->getDevice().getLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		LOG_ERROR("Failed to create image");
		throw std::runtime_error("Failed to create image");
	}
	

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_renderer->getDevice().getLogicalDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_renderer->getDevice().getPhysicalDevice());

	VkDeviceMemory imageMemory;
	if (vkAllocateMemory(m_renderer->getDevice().getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate image memory");
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(m_renderer->getDevice().getLogicalDevice(), image, imageMemory, 0);


	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = size;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_renderer->getDevice().getLogicalDevice(), &stagingBufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
		LOG_ERROR("Failed to create staging buffer for image creation");
		throw std::runtime_error("Failed to create staging buffer during image creation");
	}

	VkMemoryRequirements stagingMemoryRequirements;
	vkGetBufferMemoryRequirements(m_renderer->getDevice().getLogicalDevice(), stagingBuffer, &stagingMemoryRequirements);

	VkMemoryAllocateInfo stagingAllocationInfo{};
	stagingAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	stagingAllocationInfo.allocationSize = stagingMemoryRequirements.size;
	stagingAllocationInfo.memoryTypeIndex = findMemoryType(
		stagingMemoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_renderer->getDevice().getPhysicalDevice());

	if (vkAllocateMemory(m_renderer->getDevice().getLogicalDevice(), &stagingAllocationInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate staging buffer memory during image creation");
		throw std::runtime_error("Failed to allocate staging buffer memory during image creation");
	}

	vkBindBufferMemory(m_renderer->getDevice().getLogicalDevice(), stagingBuffer, stagingBufferMemory, 0);

	void *stagingData;
	vkMapMemory(m_renderer->getDevice().getLogicalDevice(), stagingBufferMemory, 0, size, 0, &stagingData);
	memcpy(stagingData, data, size);
	vkUnmapMemory(m_renderer->getDevice().getLogicalDevice(), stagingBufferMemory);


	transitionImageLayout(image,
		static_cast<VkFormat>(format),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		*m_renderer);

	VkCommandBuffer commandBuffer = m_renderer->prepareSingleCommand();
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
	m_renderer->executeSingleCommand(commandBuffer);

	transitionImageLayout(image,
		static_cast<VkFormat>(format),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		*m_renderer);

	vkDestroyBuffer(m_renderer->getDevice().getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_renderer->getDevice().getLogicalDevice(), stagingBufferMemory, nullptr);

	return Image(
		width,
		height,
		format,
		image,
		imageMemory
	);
}

Texture AssetManager::loadTexture(const Image &image, const TextureProperties &properties) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image.getImage();
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = static_cast<VkFormat>(image.getFormat());
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(m_renderer->getDevice().getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		LOG_ERROR("Failed to create texture image view");
		throw std::runtime_error("Failed to create texture image view");
	}


	VkPhysicalDeviceProperties physicalProperties{};
	vkGetPhysicalDeviceProperties(m_renderer->getDevice().getPhysicalDevice(), &physicalProperties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = properties.mag;
	samplerInfo.minFilter = properties.min;
	samplerInfo.addressModeU = properties.wrapU;
	samplerInfo.addressModeV = properties.wrapV;
	samplerInfo.addressModeW = properties.wrapW;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = physicalProperties.limits.maxSamplerAnisotropy;
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
	if (vkCreateSampler(m_renderer->getDevice().getLogicalDevice(), &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
		LOG_ERROR("Failed to create texture sampler");
		throw std::runtime_error("Failed to create texture sampler");
	}

	return Texture(
		properties,
		imageView,
		imageSampler
	);
}

std::unique_ptr<ModelSource> AssetManager::loadModelSource(const std::string &path) {
	return convertToModelSource(path);
}

std::unique_ptr<Model> AssetManager::loadModel(const std::unique_ptr<ModelSource> &modelSource) {
	return loadModel(*modelSource.get());
}

std::unique_ptr<Model> AssetManager::loadModel(const ModelSource &modelSource) {
	VkDeviceMemory vertexStagingMemory, modelVertexMemory;

	VkBuffer vertexStagingBuffer = createEmptyBuffer(
		modelSource.getVertexData().size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		m_renderer->getDevice().getLogicalDevice());

	VkBuffer vertexBuffer = createEmptyBuffer(
		modelSource.getVertexData().size(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		m_renderer->getDevice().getLogicalDevice());


	VkMemoryRequirements stagingMemoryRequirements;
	vkGetBufferMemoryRequirements(m_renderer->getDevice().getLogicalDevice(), vertexStagingBuffer, &stagingMemoryRequirements);

	VkMemoryAllocateInfo vertexStagingAllocationInfo{};
	vertexStagingAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vertexStagingAllocationInfo.allocationSize = stagingMemoryRequirements.size;
	vertexStagingAllocationInfo.memoryTypeIndex = findMemoryType(
		stagingMemoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_renderer->getDevice().getPhysicalDevice());

	if (vkAllocateMemory(m_renderer->getDevice().getLogicalDevice(), &vertexStagingAllocationInfo, nullptr, &vertexStagingMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate staging buffer memory");
		throw std::runtime_error("Failed to allocate staging buffer memory");
	}

	vkBindBufferMemory(m_renderer->getDevice().getLogicalDevice(), vertexStagingBuffer, vertexStagingMemory, 0);


	VkMemoryRequirements vertexMemoryRequirements;
	vkGetBufferMemoryRequirements(m_renderer->getDevice().getLogicalDevice(), vertexBuffer, &vertexMemoryRequirements);

	VkMemoryAllocateInfo veretxAllocationInfo{};
	veretxAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	veretxAllocationInfo.allocationSize = vertexMemoryRequirements.size;
	veretxAllocationInfo.memoryTypeIndex = findMemoryType(
		vertexMemoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_renderer->getDevice().getPhysicalDevice());

	if (vkAllocateMemory(m_renderer->getDevice().getLogicalDevice(), &veretxAllocationInfo, nullptr, &modelVertexMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate buffer memory");
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	vkBindBufferMemory(m_renderer->getDevice().getLogicalDevice(), vertexBuffer, modelVertexMemory, 0);


	void *data;
	vkMapMemory(m_renderer->getDevice().getLogicalDevice(), vertexStagingMemory, 0, modelSource.getVertexData().size(), 0, &data);
	memcpy(data, modelSource.getVertexData().data(), modelSource.getVertexData().size());
	vkUnmapMemory(m_renderer->getDevice().getLogicalDevice(), vertexStagingMemory);

	VkCommandBuffer commandBuffer = m_renderer->prepareSingleCommand();
	VkBufferCopy copyRegion{};
	copyRegion.size = modelSource.getVertexData().size();
	vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer, vertexBuffer, 1, &copyRegion);
	m_renderer->executeSingleCommand(commandBuffer);
	vkDestroyBuffer(m_renderer->getDevice().getLogicalDevice(), vertexStagingBuffer, nullptr);
	vkFreeMemory(m_renderer->getDevice().getLogicalDevice(), vertexStagingMemory, nullptr);


	std::vector<Image> images;
	std::vector<Material> materials;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_renderer->getDevice().getPhysicalDevice(), &properties);

	for (const auto &material : modelSource.getMaterials()) {
		Image colorImage, metallicRoughnessImage, normalImage, occlusionImage, emissiveImage;

		Texture colorTexture = loadTexture(modelSource.getTextures()[material.colorTexture], properties, modelSource, colorImage);
		Texture metallicRoughnessTexture = material.metallicRoughnessTexture == -1 ? m_defaultMetallicRoughnessTexture : loadTexture(modelSource.getTextures()[material.metallicRoughnessTexture], properties, modelSource, metallicRoughnessImage);
		Texture normalTexture = material.normalTexture == -1 ? m_defaultNormalTexture : loadTexture(modelSource.getTextures()[material.normalTexture], properties, modelSource, normalImage);
		Texture occlusionTexture = material.occlusionTexture == -1 ? m_defaultOcclusionTexture : loadTexture(modelSource.getTextures()[material.occlusionTexture], properties, modelSource, occlusionImage);
		Texture emissiveTexture = material.emissiveTexture == -1 ? m_defaultEmissiveTexture : loadTexture(modelSource.getTextures()[material.emissiveTexture], properties, modelSource, emissiveImage);

		images.push_back(colorImage);
		images.push_back(metallicRoughnessImage);
		images.push_back(normalImage);
		images.push_back(occlusionImage);
		images.push_back(emissiveImage);

		std::vector<UniformBuffer<MaterialProperties>> propertiesBuffers;
		for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; ++i) {
			propertiesBuffers.emplace_back(m_renderer->getDevice(), &material.properties);
		}

		materials.emplace_back(
			colorTexture,
			metallicRoughnessTexture,
			normalTexture,
			occlusionTexture,
			emissiveTexture,
			propertiesBuffers
		);

		m_renderer->initializeMaterials(materials.back());
	}


	return std::make_unique<Model>(
		modelVertexMemory,
		vertexBuffer,
		m_renderer->getDevice().getLogicalDevice(),
		modelSource.getMeshes(),
		images,
		materials);
}

Texture AssetManager::loadTexture(const ModelTextureData &texture, VkPhysicalDeviceProperties properties, const ModelSource &modelSource, Image &image) {
	const ModelImageData &imageSource = modelSource.getImages().at(texture.image);
	const std::byte *imageData = modelSource.getImageData().data() + imageSource.offset;

	image = loadImage(const_cast<std::byte*>(imageData), imageSource.size, imageSource.width, imageSource.height, texture.format);
	return loadTexture(image, texture.properties);
}
