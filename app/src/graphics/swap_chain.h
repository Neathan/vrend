#pragma once

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "device.h"
#include "render_pass.h"


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain {
public:

	void init(const Device& device, VkSurfaceKHR surface, GLFWwindow *window);
	void createFrameBuffers(const RenderPass& renderPass);
	void destroy();

	VkSwapchainKHR getSwapChain() const { return m_swapChain; }
	VkExtent2D getExtent() const { return m_extent; }
	VkFormat getImageFormat() const { return m_imageFormat; }

	const std::vector<VkImage> &getImages() const { return m_images; }
	const std::vector<VkImageView> &getImageViews() const { return m_imageViews; }
	const std::vector<VkFramebuffer> &getFramebuffers() const { return m_framebuffers; }

private:
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
	VkPresentModeKHR selectPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const;
	VkExtent2D selectExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) const;

	Device m_device;

	VkSwapchainKHR m_swapChain;
	VkExtent2D m_extent;
	VkFormat m_imageFormat;
	std::vector<VkImage> m_images;

	std::vector<VkImageView> m_imageViews;
	std::vector<VkFramebuffer> m_framebuffers;

	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;
};
