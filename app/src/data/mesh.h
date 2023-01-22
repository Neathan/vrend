#pragma once

#include <glad/vulkan.h>

#include <array>


struct Mesh {
	VkDeviceSize positionStart;
	VkDeviceSize positionLength;
	VkDeviceSize textureCoordinateStart;
	VkDeviceSize textureCoordinateLength;
	VkDeviceSize normalStart;
	VkDeviceSize normalLength;
	VkDeviceSize indexStart;
	VkDeviceSize indexLength;

	size_t indexCount;

	std::array<VkDeviceSize, 1> getVertexOffsets() const { return { positionStart }; }
	size_t getIndexOffset() const { return indexStart; }

	size_t getIndexCount() const { return indexCount; }
};
