#pragma once

// #include <vector>
// 
// #include "graphics/device.h"
// #include "graphics/uniform.h"
// 
// #include "graphics/material.h"
// 
// 
// #define SCENE_MAX_OBJECTS 10
// 
// 
// class DescriptorManager {
// public:
// 	void destroy();
// 
// 	void init(int count, const Device& device);
// 
// 	void (std::vector<Material> &materials, int frame);
// 
// 	const std::vector<VkDescriptorSet> &getSets() const { return m_sets; }
// 	VkDescriptorSetLayout getLayout() const { return m_layout; }
// 
// 	UniformData *getUniformData(int frame) { return m_uniformBuffers[frame].getData(); }
// 
// private:
// 	VkDescriptorSetLayout m_layout;
// 
// 	std::vector<UniformBuffer> m_uniformBuffers;
// 	std::vector<VkDescriptorSet> m_sets;
// 
// 	VkDescriptorPool m_pool;
// 
// 	VkDevice m_device;
// };
