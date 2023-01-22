#pragma once

#include <glad/vulkan.h>

#include <memory>
#include <string>
#include <array>
#include <unordered_map>

#include "data/mesh.h"
#include "data/model_source.h"
#include "graphics/renderer.h"


class Model {
public:
	static std::array<VkVertexInputBindingDescription, 1> getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();

	static std::unique_ptr<Model> load(const ModelSource& modelSource, const Renderer &renderer);

	Model(VkDeviceMemory modelMemory, VkBuffer vertexBuffer, VkDevice device,
		const std::unordered_map<int, std::vector<Mesh>>& meshes)
		: m_modelMemory(modelMemory), m_vertexBuffer(vertexBuffer), m_device(device),
		m_meshes(meshes) {}
	~Model();

	VkBuffer getVertexBuffer() const { return m_vertexBuffer; }
	const std::unordered_map<int, std::vector<Mesh>> &getMeshes() const { return m_meshes; }
private:
	VkDeviceMemory m_modelMemory;
	VkBuffer m_vertexBuffer;
	VkDevice m_device;

	std::unordered_map<int, std::vector<Mesh>> m_meshes;
};
