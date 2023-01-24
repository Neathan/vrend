#pragma once

#include <glad/vulkan.h>

VkFilter convertGLFilterToVulkan(int value);
VkSamplerAddressMode convertGLWrapModeToVulkan(int value);
