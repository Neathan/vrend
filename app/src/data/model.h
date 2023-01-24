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
	static std::array<VkVertexInputBindingDescription, 3> getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

	static std::unique_ptr<Model> load(const ModelSource& modelSource, const Renderer &renderer);

	Model(VkDeviceMemory modelMemory,
		VkBuffer vertexBuffer,
		VkDevice device,
		const std::unordered_map<int, std::vector<Mesh>>& meshes,
		const std::vector<VkImage> &images,
		const std::vector<VkDeviceMemory> &imageMemory,
		const std::vector<VkImageView> &imageViews,
		const std::vector<VkSampler> &imageSamplers)
		: m_modelMemory(modelMemory),
		m_vertexBuffer(vertexBuffer),
		m_device(device),
		m_meshes(meshes),
		m_images(images),
		m_imageMemory(imageMemory),
		m_imageViews(imageViews),
		m_imageSamplers(imageSamplers) {}

	~Model();  // TODO: Replace with destroy function

	VkBuffer getVertexBuffer() const { return m_vertexBuffer; }
	std::array<VkBuffer, 3> getVertexBufferAsArray() const { return { m_vertexBuffer, m_vertexBuffer, m_vertexBuffer }; }

	const std::unordered_map<int, std::vector<Mesh>> &getMeshes() const { return m_meshes; }

	const std::vector<VkImageView> &getImageViews() const { return m_imageViews; }
	const std::vector<VkSampler> &getImageSamplers() const { return m_imageSamplers; }

private:
	VkDeviceMemory m_modelMemory;
	VkBuffer m_vertexBuffer;
	VkDevice m_device;

	std::unordered_map<int, std::vector<Mesh>> m_meshes;

	std::vector<VkImage> m_images;
	std::vector<VkDeviceMemory> m_imageMemory;
	std::vector<VkImageView> m_imageViews;

	std::vector<VkSampler> m_imageSamplers;
};
