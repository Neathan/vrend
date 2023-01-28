#pragma once

#include <glad/vulkan.h>

#include <vector>

#include "graphics/texture.h"

struct MaterialData {
	int albedo;
	int normal;
};

class Material {
public:
	void destroy(VkDevice device);

	Texture albedo;
	Texture normal;

	std::vector<VkDescriptorSet> sets;
};
