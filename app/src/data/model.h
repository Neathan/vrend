#pragma once

#include <glad/vulkan.h>

#include <memory>
#include <string>
#include <array>
#include <unordered_map>

#include "data/mesh.h"
#include "data/model_source.h"
#include "graphics/renderer.h"
#include "graphics/material.h"


class Model {
public:
	static std::array<VkVertexInputBindingDescription, 3> getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

	Model() {}
	Model(VkDeviceMemory modelMemory,
		VkBuffer vertexBuffer,
		VkDevice device,
		std::unordered_map<int, std::vector<Mesh>> meshes,
		std::vector<Image> images,
		std::vector<Material> materials)
		: m_modelMemory(modelMemory),
		m_vertexBuffer(vertexBuffer),
		m_device(device),
		m_meshes(meshes),
		m_images(images),
		m_materials(materials) {}

	~Model(); // TODO: Replace with destroy

	VkBuffer getVertexBuffer() const { return m_vertexBuffer; }
	std::array<VkBuffer, 3> getVertexBufferAsArray() const { return { m_vertexBuffer, m_vertexBuffer, m_vertexBuffer }; }

	const std::unordered_map<int, std::vector<Mesh>> &getMeshes() const { return m_meshes; }

	std::vector<Material> &getMaterials() { return m_materials; }
	const std::vector<Material> &getMaterials() const { return m_materials; }

private:
	VkDeviceMemory m_modelMemory = VK_NULL_HANDLE;
	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	std::unordered_map<int, std::vector<Mesh>> m_meshes;

	std::vector<Image> m_images;
	std::vector<Material> m_materials;
};

struct ModelComponent {
	std::shared_ptr<Model> model;
};