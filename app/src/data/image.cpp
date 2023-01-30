#include "image.h"

void Image::destroy(VkDevice device) {
	vkDestroyImage(device, m_image, nullptr);
	vkFreeMemory(device, m_memory, nullptr);
}
