#include "texture.h"


void Texture::destroy(VkDevice device) {
	vkDestroySampler(device, sampler, nullptr);
	vkDestroyImageView(device, view, nullptr);
}
