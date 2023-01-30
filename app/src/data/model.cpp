#include "model.h"

#include <tiny_gltf.h>

#include <stdexcept>
#include <unordered_map>

#include "log.h"
#include "graphics/memory.h"

#include "data/asset_manager.h"


std::array<VkVertexInputBindingDescription, 3> Model::getBindingDescription() {
	std::array<VkVertexInputBindingDescription, 3>  bindingDescriptions{};

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(float) * 3;
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	bindingDescriptions[1].binding = 1;
	bindingDescriptions[1].stride = sizeof(float) * 2;
	bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	bindingDescriptions[2].binding = 2;
	bindingDescriptions[2].stride = sizeof(float) * 3;
	bindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::array<VkVertexInputAttributeDescription, 3> Model::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = 0;

	attributeDescriptions[1].binding = 1;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = 0;

	attributeDescriptions[2].binding = 2;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = 0;

	return attributeDescriptions;
}

Model::~Model() {
	for (auto &material : m_materials) {
		material.destroy(m_device);
	}

	for (auto &image : m_images) {
		image.destroy(m_device);
	}

	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);

	vkFreeMemory(m_device, m_modelMemory, nullptr);
}
