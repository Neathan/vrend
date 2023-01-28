#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <cstddef>

#include "data/mesh.h"
#include "graphics/image.h"
#include "graphics/texture.h"


// Stores model data in engine format
class ModelSource {
public:
	ModelSource(const std::vector<std::byte> &vertexData,
		const std::vector<std::byte> &imageData,
		const std::unordered_map<int, std::vector<Mesh>> &meshes,
		const std::vector<Image> &images,
		const std::vector<TextureData> &textures,
		const std::vector<MaterialData> &materials)
		: m_vertexData(vertexData), m_imageData(imageData), m_meshes(meshes),
		m_images(images), m_textures(textures), m_materials(materials) {}

	const std::vector<std::byte> &getVertexData() const { return m_vertexData; }
	const std::vector<std::byte> &getImageData() const { return m_imageData; }

	const std::vector<Image> &getImages() const { return m_images; }
	const std::vector<TextureData> &getTextures() const { return m_textures; }
	const std::vector<MaterialData> &getMaterials() const { return m_materials; }

	const std::unordered_map<int, std::vector<Mesh>> &getMeshes() const { return m_meshes; }
private:
	std::vector<std::byte> m_vertexData;
	std::vector<std::byte> m_imageData;

	std::unordered_map<int, std::vector<Mesh>> m_meshes;

	std::vector<Image> m_images;
	std::vector<TextureData> m_textures;
	std::vector<MaterialData> m_materials;
};
