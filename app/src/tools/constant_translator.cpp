#include "constant_translator.h"

#include <stdexcept>

#include "log.h"


VkFilter convertGLFilterToVulkan(int value) {
	switch (value) {
	case 9728: // GL_NEAREST
		return VK_FILTER_NEAREST;
	case 9729: // GL_LINEAR
		return VK_FILTER_LINEAR;

	case 9986: // GL_NEAREST_MIPMAP_LINEAR
		LOG_WARN("Unssuported filter 'GL_NEAREST_MIPMAP_LINEAR' used. Using GL_LINEAR instead.");
		return VK_FILTER_LINEAR;
	case 9987: // GL_LINEAR_MIPMAP_LINEAR
		LOG_WARN("Unssuported filter 'GL_LINEAR_MIPMAP_LINEAR' used. Using GL_LINEAR instead.");
		return VK_FILTER_LINEAR;

	case -1:
		LOG_DEBUG("OpenGL filter constant -1 was converted to VK_FILTER_LINEAR");
		return VK_FILTER_LINEAR;

	// TODO: Implement remaining possible values
	default:
		LOG_ERROR("Failed to convert OpenGL filter value to vulkan. Value: {}", value);
		throw std::runtime_error("Failed to convert OpenGL filter value to vulkan");
	}
}

VkSamplerAddressMode convertGLWrapModeToVulkan(int value) {
	switch (value) {
	case 10497: // GL_REPEAT
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case 33648: // GL_MIRRORED_REPEAT
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case 33071: // GL_CLAMP_TO_EDGE
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case 33069: // GL_CLAMP_TO_BORDER
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case 34627: // GL_MIRROR_CLAMP_TO_EDGE 
		return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

	case -1:
		LOG_DEBUG("OpenGL wrap mode constant -1 was converted to VK_SAMPLER_ADDRESS_MODE_REPEAT");
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;

	default:
		LOG_ERROR("Failed to convert OpenGL wrap mode value to vulkan. Value: {}", value);
		throw std::runtime_error("Failed to convert OpenGL wrap mode value to vulkan");
	}
}

size_t componentByteSize(int value) {
	switch (value) {
	case 5120:
	case 5121: return 1;
	case 5122:
	case 5123: return 2;
	case 5124:
	case 5125:
	case 5126: return 4;
	case 5130: return 8;
	default:
		LOG_ERROR("Failed to convert OpenGL component type to byte size. Value: {}", value);
		throw std::runtime_error("Failed to convert OpenGL component type to byte size");
	}
}

size_t componentTypeComponents(int value) {
#define TINYGLTF_TYPE_VEC2 (2)
#define TINYGLTF_TYPE_VEC3 (3)
#define TINYGLTF_TYPE_VEC4 (4)
#define TINYGLTF_TYPE_MAT2 (32 + 2)
#define TINYGLTF_TYPE_MAT3 (32 + 3)
#define TINYGLTF_TYPE_MAT4 (32 + 4)
#define TINYGLTF_TYPE_SCALAR (64 + 1)
#define TINYGLTF_TYPE_VECTOR (64 + 4)
#define TINYGLTF_TYPE_MATRIX (64 + 16)

	switch (value) {
	case 2: return 2;
	case 3: return 3;
	case 4: return 4;
	case 32 + 2: return 2 * 2;
	case 32 + 3: return 3 * 3;
	case 32 + 4: return 4 * 4;
	case 64 + 1: return 1;
// 	case 64 + 4: return 4;
// 	case 64 + 16: return 4 * 4;
	default:
		LOG_ERROR("Failed to convert OpenGL component type to component count. Value: {}", value);
		throw std::runtime_error("Failed to convert OpenGL component type to component count");
	}
}
