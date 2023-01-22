#pragma once

#include <glad/vulkan.h>

#include "device.h"

class RenderPass {
public:
	void init(const Device &device, VkFormat swapChainImageFormat);
	void destroy();

	VkRenderPass getRenderPass() const { return m_renderPass; }

private:
	Device m_device;

	VkRenderPass m_renderPass;

};
