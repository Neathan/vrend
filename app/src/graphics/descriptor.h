#pragma once

#include <vector>

#include "graphics/device.h"
#include "graphics/uniform.h"


class DescriptorManager {
public:
	void destroy();

	void setLayout(VkDescriptorSetLayout layout) { m_layout = layout; }
	void init(int count, const Device& device);

	void setSamplerDescriptorSet(VkImageView imageView, VkSampler imageSampler, int frame);

	const std::vector<VkDescriptorSet> &getSets() const { return m_sets; }

	UniformData *getUniformData(int frame) { return m_uniformBuffers[frame].getData(); }

private:
	VkDescriptorSetLayout m_layout;

	std::vector<UniformBuffer> m_uniformBuffers;
	std::vector<VkDescriptorSet> m_sets;

	VkDescriptorPool m_pool;

	VkDevice m_device;
};
