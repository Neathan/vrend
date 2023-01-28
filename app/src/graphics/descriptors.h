#pragma once

// Note: Implementation based on: https://github.com/vblanco20-1/vulkan-guide/blob/engine/extra-engine/vk_descriptors.h
// Found at: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
// Thread safe version available here: https://github.com/vblanco20-1/Vulkan-Descriptor-Allocator

#include <glad/vulkan.h>

#include <vector>
#include <array>
#include <unordered_map>

#define DESCRIPTOR_ALLOCATOR_BATCH_SIZE 1000

class DescriptorAllocator {
public:
	struct PoolSizes {
		// TODO: Tweak these to fit the application
		std::vector<std::pair<VkDescriptorType, float>> sizes = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
		};
	};

	void resetPools();
	void allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout);

	void init(VkDevice device);
	void destroy();

	VkDevice getDevice() const { return m_device; }

private:
	VkDescriptorPool grabPool();

	VkDevice m_device;
	VkDescriptorPool m_currentPool{ VK_NULL_HANDLE };
	PoolSizes m_descriptorSizes;
	std::vector<VkDescriptorPool> m_usedPools;
	std::vector<VkDescriptorPool> m_freePools;
};

class DescriptorLayoutCache {
public:
	void init(VkDevice device);
	void destroy();

	VkDescriptorSetLayout createDescriptorLayout(VkDescriptorSetLayoutCreateInfo *info);

	struct DescriptorLayoutInfo {
		// TODO: Note from source implementation: "good idea to turn this into a inlined array"
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		bool operator==(const DescriptorLayoutInfo &other) const;
		size_t hash() const;
	};

private:
	struct DescriptorLayoutHash {
		std::size_t operator()(const DescriptorLayoutInfo &k) const {
			return k.hash();
		}
	};

	VkDevice m_device;
	std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_layoutCache;
};

class DescriptorBuilder {
public:
	static DescriptorBuilder begin(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator);

	DescriptorBuilder &bindBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
	DescriptorBuilder &bindImage(uint32_t binding, VkDescriptorImageInfo *imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

	DescriptorBuilder &bindDummy(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags);

	void build(VkDescriptorSet &set, VkDescriptorSetLayout &layout);
	void build(VkDescriptorSet &set);

	void buildLayout(VkDescriptorSetLayout &layout);

private:
	std::vector<VkWriteDescriptorSet> m_writes;
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;

	DescriptorLayoutCache *m_cache;
	DescriptorAllocator *m_allocator;
};

