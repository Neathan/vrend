#include "material.h"


void Material::destroy(VkDevice device) {
	if(!colorTexture.isDefault()) colorTexture.destroy(device);
	if (!metallicRoughnessTexture.isDefault()) metallicRoughnessTexture.destroy(device);
	if (!normalTexture.isDefault()) normalTexture.destroy(device);
	if (!occlusionTexture.isDefault()) occlusionTexture.destroy(device);
	if (!emissiveTexture.isDefault()) emissiveTexture.destroy(device);

	for (auto &buffer : propertiesBuffers) {
		buffer.destroy();
	}

	// Sets lifetime is not managed by Material
}