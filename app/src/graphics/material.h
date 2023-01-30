#pragma once

#include <glad/vulkan.h>
#include <glm/glm.hpp>

#include <vector>

#include "data/texture.h"
#include "graphics/uniform.h"


struct MaterialProperties {
	glm::vec4 colorFactor;
	float metallicFactor;
	float roughnessFactor;
	glm::vec4 emissiveFactor; // Note: Actually vec3 but requires vec4 for alignment
	bool dubbleSided;
};

class Material {
public:
	Material(Texture colorTexture,
		Texture metallicRoughnessTexture,
		Texture normalTexture,
		Texture occlusionTexture,
		Texture emissiveTexture,
		std::vector<UniformBuffer<MaterialProperties>> propertiesBuffers = {})
		: colorTexture(colorTexture),
		metallicRoughnessTexture(metallicRoughnessTexture),
		normalTexture(normalTexture),
		occlusionTexture(occlusionTexture),
		emissiveTexture(emissiveTexture),
		propertiesBuffers(propertiesBuffers),
		sets() {}

	void destroy(VkDevice device);
	MaterialProperties *getProperties(int index) { return propertiesBuffers[index].getData(); }

	Texture colorTexture;
	Texture metallicRoughnessTexture;

	Texture normalTexture;
	Texture occlusionTexture;
	Texture emissiveTexture;

	std::vector<UniformBuffer<MaterialProperties>> propertiesBuffers;
	std::vector<VkDescriptorSet> sets;
};
