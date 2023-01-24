#pragma once

#include <glad/vulkan.h>

struct Texture {
	int image;

	VkFilter mag;
	VkFilter min;
	VkSamplerAddressMode wrapU;
	VkSamplerAddressMode wrapV;
	VkSamplerAddressMode wrapW;
};
