#pragma once

#include <glad/vulkan.h>


struct TextureProperties {
	VkFilter mag = VK_FILTER_LINEAR;
	VkFilter min = VK_FILTER_LINEAR;
	VkSamplerAddressMode wrapU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode wrapV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode wrapW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

class Texture {
public:
	Texture() {}
	Texture(TextureProperties properties, VkImageView view, VkSampler sampler)
		: m_properties(properties), m_view(view), m_sampler(sampler) {}

	void destroy(VkDevice device);

	void setDefault(bool state) { m_default = state; }
	bool isDefault() const { return m_default; }

	VkImageView getView() const { return m_view; }
	VkSampler getSampler() const { return m_sampler; }
private:
	TextureProperties m_properties{};

	VkImageView m_view = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;

	bool m_default = false;
};
