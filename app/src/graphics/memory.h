#pragma once

#include <glad/vulkan.h>

#include "graphics/device.h"


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const Device &device);
