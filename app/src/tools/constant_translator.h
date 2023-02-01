#pragma once

#include <glad/vulkan.h>

VkFilter convertGLFilterToVulkan(int value);
VkSamplerAddressMode convertGLWrapModeToVulkan(int value);

size_t componentByteSize(int value);
size_t componentTypeComponents(int value);
