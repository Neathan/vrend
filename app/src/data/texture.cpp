#include "texture.h"

void Texture::destroy(VkDevice device) {
	vkDestroyImageView(device, m_view, nullptr);
	vkDestroySampler(device, m_sampler, nullptr);
}
