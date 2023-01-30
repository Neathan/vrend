#pragma once

#include <glad/vulkan.h>


enum class ImageFormat {
	SRGB = VK_FORMAT_R8G8B8A8_SRGB,
	LINEAR = VK_FORMAT_R8G8B8A8_UNORM
};

class Image {
public:
	Image() {}
	Image(int width, int height, ImageFormat format, VkImage image, VkDeviceMemory memory)
		: m_width(width), m_height(height), m_format(format), m_image(image), m_memory(memory) {}

	void destroy(VkDevice device);

	ImageFormat getFormat() const { return m_format; }
	VkImage getImage() const { return m_image; }
	VkDeviceMemory getMemory() const { return m_memory; }
private:
	int m_width = -1;
	int m_height = -1;

	ImageFormat m_format = ImageFormat::SRGB;

	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
};
