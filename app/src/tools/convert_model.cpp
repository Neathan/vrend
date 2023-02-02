#include "convert_model.h"

#include <tiny_gltf.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "data/image.h"
#include "data/texture.h"
#include "tools/constant_translator.h"
#include "tools/convert_vector.h"
#include "log.h"

std::vector<std::byte> extractData(const tinygltf::Buffer &buffer, const tinygltf::BufferView &bufferView, size_t bufferViewOffset, size_t size) {
	const std::byte *base = reinterpret_cast<const std::byte *>(buffer.data.data()) + bufferViewOffset + bufferView.byteOffset;
	return std::vector<std::byte>(base, base + size);
}

size_t accessorByteSize(const tinygltf::Accessor &accessor) {
	size_t componentSize = componentByteSize(accessor.componentType);
	size_t componentTypeCount = componentTypeComponents(accessor.type);
	return componentSize * componentTypeCount * accessor.count;
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
				positionData = extractData(buffer, bufferView, accessor.byteOffset, accessorByteSize(accessor));
			else if (key == "TEXCOORD_0")
				textureCoordinateData = extractData(buffer, bufferView, accessor.byteOffset, accessorByteSize(accessor));
			else if (key == "NORMAL")
				normalData = extractData(buffer, bufferView, accessor.byteOffset, accessorByteSize(accessor));
		}

		const auto &indicesAccessor = model.accessors[primitive.indices];
		const auto &indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
		indexData = extractData(model.buffers[indicesBufferView.buffer], indicesBufferView, indicesAccessor.byteOffset, accessorByteSize(indicesAccessor));

		// TODO: Make sure indicesAccessor.componentType is properly used

		// TODO: Extract image data

		// Make sure data is aligned
		for (size_t i = vertexData.size(); i % 4 != 0; ++i) {
			vertexData.push_back(std::byte{0});
		}

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

			indicesAccessor.count,

			primitive.material
		});
	}

	return meshes;
}

glm::mat4 getNodeTransformationMatrix(const tinygltf::Node &node) {
	if (node.matrix.size()) {
		return toMat4(node.matrix);
	}
	glm::mat4 matrix(1.0f);
	if (node.scale.size()) {
		matrix = glm::scale(matrix, toVec3(node.scale));
	}
	if (node.rotation.size()) {
		glm::vec4 rot = toVec4(node.rotation);
		matrix *= glm::mat4_cast(glm::quat(rot.x, rot.y, rot.z, rot.w));
	}
	if (node.translation.size()) {
		matrix = glm::translate(matrix, toVec3(node.translation));
	}
	return matrix;
}

void processNode(int nodeIndex, const tinygltf::Model &model, std::vector<std::byte> &vertexData,
	std::unordered_map<int, std::vector<Mesh>> &modelMeshes, std::unordered_map<int, glm::mat4> &meshMatricies,
	const glm::mat4& parentMatrix = glm::mat4(1.0f)) {

	const tinygltf::Node &node = model.nodes[nodeIndex];

	glm::mat4 transform = parentMatrix * getNodeTransformationMatrix(node);

	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		const auto &mesh = model.meshes[node.mesh];
		modelMeshes[node.mesh] = convertToModelSourceMeshes(mesh, model, vertexData);
		meshMatricies[node.mesh] = transform;
	}

	for (int child : node.children) {
		processNode(child, model, vertexData, modelMeshes, meshMatricies, transform);
	}
}

std::vector<ModelImageData> getModelImages(const tinygltf::Model& model, std::vector<std::byte>& imageData) {
	std::vector<ModelImageData> images;
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

std::vector<ModelTextureData> getModelTextures(const tinygltf::Model &model) {
	std::vector<ModelTextureData> textures;
	textures.reserve(model.textures.size());

	for (const auto &texture : model.textures) {
		// TODO: Manage missing sampler and image

		const auto &sampler = model.samplers[texture.sampler];

		textures.push_back({
			texture.source,
			ImageFormat::SRGB, // Needs to be corrected by material
			TextureProperties {
				convertGLFilterToVulkan(sampler.magFilter),
				convertGLFilterToVulkan(sampler.minFilter),
				convertGLWrapModeToVulkan(sampler.wrapS),
				convertGLWrapModeToVulkan(sampler.wrapT),
				VK_SAMPLER_ADDRESS_MODE_REPEAT
			}
		});
	}

	return textures;
}

std::vector<ModelMaterialData> getModelMaterials(const tinygltf::Model &model) {
	std::vector<ModelMaterialData> materials;
	materials.reserve(model.materials.size());

	// TODO: Normal scale and occlusion strength support

	for (const auto &material : model.materials) {
		materials.push_back({
			material.pbrMetallicRoughness.baseColorTexture.index,
			material.pbrMetallicRoughness.metallicRoughnessTexture.index,
			material.normalTexture.index,
			material.occlusionTexture.index,
			material.emissiveTexture.index,
			MaterialProperties {
				toVec4(material.pbrMetallicRoughness.baseColorFactor),
				static_cast<float>(material.pbrMetallicRoughness.metallicFactor),
				static_cast<float>(material.pbrMetallicRoughness.roughnessFactor),
				glm::vec4(toVec3(material.emissiveFactor), -1),
				material.doubleSided
			}
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
	std::unordered_map<int, glm::mat4> meshMatricies;

	const auto &scene = model.scenes[model.defaultScene];

	for (size_t nodeIndex = 0; nodeIndex < scene.nodes.size(); ++nodeIndex) {
		processNode(scene.nodes[nodeIndex], model, vertexData, modelMeshes, meshMatricies);
	}

	std::vector<std::byte> imageData;
	std::vector<ModelImageData> images = getModelImages(model, imageData);

	std::vector<ModelTextureData> textures = getModelTextures(model);
	std::vector<ModelMaterialData> materials = getModelMaterials(model);

	for (const auto &material : materials) {
		if (material.metallicRoughnessTexture != -1)
			textures[material.metallicRoughnessTexture].format = ImageFormat::LINEAR;
		if (material.normalTexture != -1)
			textures[material.normalTexture].format = ImageFormat::LINEAR;
		if (material.occlusionTexture != -1)
			textures[material.occlusionTexture].format = ImageFormat::LINEAR;
	}

	return std::make_unique<ModelSource>(vertexData, imageData, modelMeshes, meshMatricies, images, textures, materials);
}
