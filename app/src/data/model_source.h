#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <cstddef>

#include "data/mesh.h"


// Stores model data in engine format
class ModelSource {
public:
	ModelSource(const std::vector<std::byte> &vertexData,
		const std::vector<std::byte> &imageData,
		const std::unordered_map<int, std::vector<Mesh>> &meshes)
		: m_vertexData(vertexData), m_imageData(imageData), m_meshes(meshes) {}

	const std::vector<std::byte> getVertexData() const { return m_vertexData; }
	const std::vector<std::byte> getImageData() const { return m_imageData; }

	const std::unordered_map<int, std::vector<Mesh>> &getMeshes() const { return m_meshes; }
private:
	std::vector<std::byte> m_vertexData;
	std::vector<std::byte> m_imageData;

	std::unordered_map<int, std::vector<Mesh>> m_meshes;
};
