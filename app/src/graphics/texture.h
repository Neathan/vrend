#pragma once

#include <glad/vulkan.h>

struct TextureData {
	int image;

	VkFilter mag;
	VkFilter min;
	VkSamplerAddressMode wrapU;
	VkSamplerAddressMode wrapV;
	VkSamplerAddressMode wrapW;
};

struct Texture {
	VkImageView view;
	VkSampler sampler;

	void destroy(VkDevice device);
};
