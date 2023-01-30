#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <cstddef>

#include "data/mesh.h"
#include "data/image.h"
#include "data/texture.h"
#include "graphics/material.h"


struct ModelImageData {
	int width;
	int height;
	int bits;

	size_t offset;
	size_t size;
};

struct ModelTextureData {
	int image;
	ImageFormat format;

	TextureProperties properties;
};

struct ModelMaterialData {
	int colorTexture;
	int metallicRoughnessTexture;
	int normalTexture;
	int occlusionTexture;
	int emissiveTexture;

	MaterialProperties properties;
};


// Stores model data in engine format
class ModelSource {
public:
	ModelSource(const std::vector<std::byte> &vertexData,
		const std::vector<std::byte> &imageData,
		const std::unordered_map<int, std::vector<Mesh>> &meshes,
		const std::vector<ModelImageData> &images,
		const std::vector<ModelTextureData> &textures,
		const std::vector<ModelMaterialData> &materials)
		: m_vertexData(vertexData), m_imageData(imageData), m_meshes(meshes),
		m_images(images), m_textures(textures), m_materials(materials) {}

	const std::vector<std::byte> &getVertexData() const { return m_vertexData; }
	const std::vector<std::byte> &getImageData() const { return m_imageData; }

	const std::vector<ModelImageData> &getImages() const { return m_images; }
	const std::vector<ModelTextureData> &getTextures() const { return m_textures; }
	const std::vector<ModelMaterialData> &getMaterials() const { return m_materials; }

	const std::unordered_map<int, std::vector<Mesh>> &getMeshes() const { return m_meshes; }
private:
	std::vector<std::byte> m_vertexData;
	std::vector<std::byte> m_imageData;

	std::unordered_map<int, std::vector<Mesh>> m_meshes;

	std::vector<ModelImageData> m_images;
	std::vector<ModelTextureData> m_textures;
	std::vector<ModelMaterialData> m_materials;
};
