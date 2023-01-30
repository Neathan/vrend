#include "material.h"


void Material::destroy(VkDevice device) {
	colorTexture.destroy(device);
	metallicRoughnessTexture.destroy(device);
	normalTexture.destroy(device);
	occlusionTexture.destroy(device);
	emissiveTexture.destroy(device);

	for (auto &buffer : propertiesBuffers) {
		buffer.destroy();
	}

	// Sets lifetime is not managed by Material
}