#include "convert_model.h"

#include <tiny_gltf.h>

#include <vector>

#include "graphics/image.h"
#include "graphics/texture.h"
#include "tools/constant_translator.h"
#include "log.h"

std::vector<std::byte> extractData(const tinygltf::Buffer &buffer, const tinygltf::BufferView &bufferView, size_t bufferViewOffset) {
	const std::byte *base = reinterpret_cast<const std::byte *>(buffer.data.data()) + bufferViewOffset + bufferView.byteOffset;
	return std::vector<std::byte>(base, base + bufferView.byteLength);
}

std::vector<Mesh> convertToModelSourceMeshes(const tinygltf::Mesh &mesh, const tinygltf::Model &model, std::vector<std::byte> &vertexData) {
	std::vector<Mesh> meshes;

	for (const auto &primitive : mesh.primitives) {
		std::vector<std::byte> positionData;
		std::vector<std::byte> textureCoordinateData;
		std::vector<std::byte> normalData;
		std::vector<std::byte> indexData;

		for (const auto &[key, value] : primitive.attributes) {
			const auto &accessor = model.accessors[value];
			const auto &bufferView = model.bufferViews[accessor.bufferView];
			const auto &buffer = model.buffers[bufferView.buffer];
			// TODO: Check for if accessor is sparse, if its tightly packed (stride = 0) and if buffer view is targeted for indexed rendering

			if (key == "POSITION")
				positionData = extractData(buffer, bufferView, accessor.byteOffset);
			else if (key == "TEXCOORD_0")
				textureCoordinateData = extractData(buffer, bufferView, accessor.byteOffset);
			else if (key == "NORMAL")
				normalData = extractData(buffer, bufferView, accessor.byteOffset);
		}

		const auto &indicesAccessor = model.accessors[primitive.indices];
		const auto &indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
		indexData = extractData(model.buffers[indicesBufferView.buffer], indicesBufferView, indicesAccessor.byteOffset);

		// TODO: Make sure indicesAccessor.componentType is properly used

		// TODO: Extract image data


		size_t baseOffset = vertexData.size();

		// Order is normalized
		vertexData.insert(vertexData.end(), positionData.begin(), positionData.end());
		vertexData.insert(vertexData.end(), textureCoordinateData.begin(), textureCoordinateData.end());
		vertexData.insert(vertexData.end(), normalData.begin(), normalData.end());
		vertexData.insert(vertexData.end(), indexData.begin(), indexData.end());

		meshes.push_back({
			baseOffset,
			positionData.size(),

			baseOffset + positionData.size(),
			textureCoordinateData.size(),

			baseOffset + positionData.size() + textureCoordinateData.size(),
			normalData.size(),

			baseOffset + positionData.size() + textureCoordinateData.size() + normalData.size(),
			indexData.size(),

			indicesAccessor.count
		});
	}

	return meshes;
}

void processNode(const tinygltf::Node &node, const tinygltf::Model &model, std::vector<std::byte> &vertexData, std::unordered_map<int, std::vector<Mesh>> &modelMeshes) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		const auto &mesh = model.meshes[node.mesh];
		modelMeshes[node.mesh] = convertToModelSourceMeshes(mesh, model, vertexData);
	}

	for (int child : node.children) {
		processNode(model.nodes[child], model, vertexData, modelMeshes);
	}
}

std::vector<Image> getModelImages(const tinygltf::Model& model, std::vector<std::byte>& imageData) {
	std::vector<Image> images;
	images.reserve(model.images.size());

	for (const auto &image : model.images) {
		// TODO: Check if image uses URI or buffer view (does "image" always resolve?)
		size_t baseOffset = imageData.size();

		const std::byte *data = reinterpret_cast<const std::byte *>(image.image.data());
		imageData.insert(imageData.end(), data, data + image.image.size());

		images.push_back({
			image.width,
			image.height,
			image.bits,
			baseOffset,
			image.image.size()
		});
	}

	return images;
}

std::vector<TextureData> getModelTextures(const tinygltf::Model &model) {
	std::vector<TextureData> textures;
	textures.reserve(model.textures.size());

	for (const auto &texture : model.textures) {
		// TODO: Manage missing sampler and image

		const auto &sampler = model.samplers[texture.sampler];

		textures.push_back({
			texture.source,
			convertGLFilterToVulkan(sampler.magFilter),
			convertGLFilterToVulkan(sampler.minFilter),
			convertGLWrapModeToVulkan(sampler.wrapS),
			convertGLWrapModeToVulkan(sampler.wrapT),
			VK_SAMPLER_ADDRESS_MODE_REPEAT
		});
	}

	return textures;
}

std::vector<MaterialData> getModelMaterials(const tinygltf::Model &model) {
	std::vector<MaterialData> materials;
	materials.reserve(model.materials.size());

	for (const auto &material : model.materials) {
		materials.push_back({
			material.pbrMetallicRoughness.baseColorTexture.index,
			material.normalTexture.index
		});
	}
	return materials;
}

std::unique_ptr<ModelSource> convertToModelSource(const std::string &path) {
	tinygltf::TinyGLTF loader;

	tinygltf::Model model;
	std::string err;
	std::string warn;
	bool res = loader.LoadBinaryFromFile(&model, &err, &warn, path);

	if (!warn.empty()) {
		LOG_WARN("GLTF: {}", warn);
	}
	if (!err.empty()) {
		LOG_ERROR("GLTF: {}", err);
	}

	if (!res) {
		LOG_ERROR("Failed to load model at: {}", path);
		return nullptr;
	}

	std::vector<std::byte> vertexData;
	std::unordered_map<int, std::vector<Mesh>> modelMeshes;

	const auto &scene = model.scenes[model.defaultScene];

	for (size_t nodeIndex = 0; nodeIndex < scene.nodes.size(); ++nodeIndex) {
		const auto &node = model.nodes[scene.nodes[nodeIndex]];

		processNode(node, model, vertexData, modelMeshes);
	}

	std::vector<std::byte> imageData;
	std::vector<Image> images = getModelImages(model, imageData);

	std::vector<TextureData> textures = getModelTextures(model);
	std::vector<MaterialData> materials = getModelMaterials(model);

	return std::make_unique<ModelSource>(vertexData, imageData, modelMeshes, images, textures, materials);
}
