#pragma once

#include <glad/vulkan.h>

#include "graphics/device.h"


void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const Device& device, VkImage &image, VkDeviceMemory &imageMemory);
