#include "material.h"


void Material::destroy(VkDevice device) {
	albedo.destroy(device);
	normal.destroy(device);
}
