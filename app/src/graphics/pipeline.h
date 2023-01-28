#pragma once

#include <glad/vulkan.h>

#include <vector>
#include <string>

#include "device.h"
#include "render_pass.h"
#include "swap_chain.h"


class Pipeline {
public:
	void init(const Device &device, const RenderPass& renderPass, const SwapChain& swapChain,
		const std::string& vertexShader, const std::string& fragmentShader,
		const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts);
	void destory();

	VkPipeline getPipeline() const { return m_pipeline; }
	VkPipelineLayout getLayout() const { return m_layout; }

private:
	VkShaderModule createShaderModule(const Device &device, const std::vector<char> &code);

	Device m_device;

	VkPipeline m_pipeline;
	VkPipelineLayout m_layout;
};
