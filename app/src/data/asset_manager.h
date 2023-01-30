#pragma once

#include <string>

#include "data/image.h"
#include "data/texture.h"
#include "data/model_source.h"
#include "data/model.h"


class Renderer;

class AssetManager {
public:
	AssetManager(Renderer *renderer);

	void destroy();

	Image loadImage(const std::string &path, ImageFormat format);
	Image loadImage(void *data, size_t size, unsigned int width, unsigned int height, ImageFormat format);
	Texture loadTexture(const Image &image, const TextureProperties &properties);
	std::unique_ptr<ModelSource> loadModelSource(const std::string &path);
	std::unique_ptr<Model> loadModel(const std::unique_ptr<ModelSource> &modelSource);
	std::unique_ptr<Model> loadModel(const ModelSource &modelSource);

private:
	Texture loadTexture(const ModelTextureData &texture, VkPhysicalDeviceProperties properties, const ModelSource &modelSource, Image &image);

	Renderer *m_renderer;

	// Default textures
	Image m_defaultMetallicRoughnessImage;
	Image m_defaultNormalImage;
	Image m_defaultOcclusionImage;
	Image m_defaultEmissiveImage;
	Texture m_defaultMetallicRoughnessTexture;
	Texture m_defaultNormalTexture;
	Texture m_defaultOcclusionTexture;
	Texture m_defaultEmissiveTexture;
};