#include "swap_chain.h"

#include <algorithm>
#include <stdexcept>

#include "log.h"


void SwapChain::destroy() {
	if (!m_device) {
		return;
	}

	for (auto framebuffer : m_framebuffers) {
		vkDestroyFramebuffer(m_device.getLogicalDevice(), framebuffer, nullptr);
	}

	for (auto imageView : m_imageViews) {
		vkDestroyImageView(m_device.getLogicalDevice(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device.getLogicalDevice(), m_swapChain, nullptr);
}

void SwapChain::init(const Device &device, VkSurfaceKHR surface, GLFWwindow *window) {
	m_device = device;

	// Find limits for swap chain
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device.getPhysicalDevice(), surface);

	// Select optimal configuration based on limitations
	VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = selectPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = selectExtent(swapChainSupport.capabilities, window);

	// Define multi-buffer count
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	// Check that we are within limits
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}


	// Create swap chain from final config
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = device.findQueueFamilies(surface);
	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device.getLogicalDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
		LOG_ERROR("Failed to create swap chain");
		throw std::runtime_error("Failed to create swap chain");
	}

	vkGetSwapchainImagesKHR(device.getLogicalDevice(), m_swapChain, &imageCount, nullptr);
	m_images.resize(imageCount);
	vkGetSwapchainImagesKHR(device.getLogicalDevice(), m_swapChain, &imageCount, m_images.data());

	m_imageFormat = surfaceFormat.format;
	m_extent = extent;

	
	// Create image views
	m_imageViews.resize(m_images.size());

	for (size_t i = 0; i < m_images.size(); ++i) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_images[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_imageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.getLogicalDevice(), &createInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
			LOG_ERROR("Failed to create image view");
			throw std::runtime_error("Failed to create image view");
		}
	}
}

void SwapChain::createFrameBuffers(const RenderPass &renderPass) {
	m_framebuffers.resize(m_imageViews.size());

	for (size_t i = 0; i < m_imageViews.size(); i++) {
		VkImageView attachments[] = {
			m_imageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass.getRenderPass();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_extent.width;
		framebufferInfo.height = m_extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device.getLogicalDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
			LOG_ERROR("Failed to create framebuffer");
			throw std::runtime_error("Failed to create framebuffer");
		}
	}
}

SwapChainSupportDetails SwapChain::querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
			details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount,
			details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR SwapChain::selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const {
	for (const auto &availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SwapChain::selectPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const {
	for (const auto &availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::selectExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) const {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
