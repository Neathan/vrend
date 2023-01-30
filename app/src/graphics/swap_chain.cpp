#include "swap_chain.h"

#include <algorithm>
#include <stdexcept>
#include <array>

#include "data/image.h"
#include "graphics/memory.h"
#include "log.h"


VkMemoryRequirements createImage(uint32_t width, uint32_t height, VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	const Device &device, VkImage &image, VkDeviceMemory &imageMemory) {

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		LOG_ERROR("Failed to create image");
		throw std::runtime_error("Failed to create image");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device.getLogicalDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, device.getPhysicalDevice());

	if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate image memory");
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(device.getLogicalDevice(), image, imageMemory, 0);

	return memRequirements;
}

void SwapChain::destroy() {
	if (!m_device) {
		return;
	}

	vkDestroyImageView(m_device.getLogicalDevice(), m_depthImageView, nullptr);
	vkDestroyImage(m_device.getLogicalDevice(), m_depthImage, nullptr);
	vkFreeMemory(m_device.getLogicalDevice(), m_depthImageMemory, nullptr);

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
	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = surface;

	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = device.findQueueFamilies(surface);
	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = VK_TRUE;

	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device.getLogicalDevice(), &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
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

	// Create depth image
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT; // TODO: Make non-static (also render_pass.cpp)
	createImage(extent.width, extent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		device, m_depthImage, m_depthImageMemory);
	

	VkImageViewCreateInfo depthViewCreateInfo{};
	depthViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthViewCreateInfo.image = m_depthImage;

	depthViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthViewCreateInfo.format = depthFormat;

	depthViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	depthViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthViewCreateInfo.subresourceRange.baseMipLevel = 0;
	depthViewCreateInfo.subresourceRange.levelCount = 1;
	depthViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	depthViewCreateInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device.getLogicalDevice(), &depthViewCreateInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
		LOG_ERROR("Failed to create depth image view");
		throw std::runtime_error("Failed to create depth image view");
	}
}

void SwapChain::createFrameBuffers(const RenderPass &renderPass) {
	m_framebuffers.resize(m_imageViews.size());

	for (size_t i = 0; i < m_imageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			m_imageViews[i],
			m_depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass.getRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
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
