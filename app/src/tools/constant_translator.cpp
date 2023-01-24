#include "constant_translator.h"

#include <stdexcept>

#include "log.h"


VkFilter convertGLFilterToVulkan(int value) {
	switch (value) {
	case 9728: // GL_NEAREST
		return VK_FILTER_NEAREST;
	case 9729: // GL_LINEAR
		return VK_FILTER_LINEAR;

	case -1:
		LOG_DEBUG("OpenGL filter constant -1 was converted to VK_FILTER_LINEAR");
		return VK_FILTER_LINEAR;

	// TODO: Implement remaining possible values
	default:
		LOG_ERROR("Failed to convert OpenGL filter value to vulkan. Value: {}", value);
		throw std::runtime_error("Failed to convert OpenGL filter value to vulkan");
		break;
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
		break;
	}
}
