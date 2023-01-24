#pragma once

#include <glad/vulkan.h>

#include <vector>
#include <cstddef>

#include "graphics/device.h"
#include "graphics/renderer.h"


struct Image {
	int width;
	int height;
	int bits;

	size_t offset;
	size_t size;
};


VkMemoryRequirements createImage(uint32_t width, uint32_t height, VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	const Device& device, VkImage &image, VkDeviceMemory &imageMemory);

VkImageView loadImage(void *imageData, size_t imageSize, uint32_t width, uint32_t height,
	VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, const Renderer &renderer,
	VkImage &image, VkDeviceMemory &imageMemory);
