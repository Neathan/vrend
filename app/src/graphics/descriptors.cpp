#include "descriptors.h"

#include <stdexcept>
#include <algorithm>

#include "log.h"


VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes &poolSizes, int count, VkDescriptorPoolCreateFlags flags) {
	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(poolSizes.sizes.size());
	for (auto [size, fract] : poolSizes.sizes) {
		sizes.push_back({ size, static_cast<uint32_t>(fract * count) });
	}
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = flags;
	poolInfo.maxSets = count;
	poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
	poolInfo.pPoolSizes = sizes.data();

	VkDescriptorPool descriptorPool;
	vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

	return descriptorPool;
}

void DescriptorAllocator::resetPools() {
	for (auto pool : m_usedPools) {
		vkResetDescriptorPool(m_device, pool, 0);
	}

	m_freePools = m_usedPools;
	m_usedPools.clear();
	m_currentPool = VK_NULL_HANDLE;
}

void DescriptorAllocator::allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout) {
	if (m_currentPool == VK_NULL_HANDLE) {
		m_currentPool = grabPool();
		m_usedPools.push_back(m_currentPool);
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	
	allocInfo.pSetLayouts = &layout;
	allocInfo.descriptorPool = m_currentPool;
	allocInfo.descriptorSetCount = 1;

	VkResult allocResult = vkAllocateDescriptorSets(m_device, &allocInfo, set);

	switch (allocResult) {
	case VK_SUCCESS:
		return;
	case VK_ERROR_FRAGMENTED_POOL:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		break;
	default:
		LOG_ERROR("Failed to allocate required descriptor sets. Error: {}", allocResult);
		throw std::runtime_error("Failed to allocate required descriptor sets");
	}

	m_currentPool = grabPool();
	m_usedPools.push_back(m_currentPool);
	allocInfo.descriptorPool = m_currentPool;

	allocResult = vkAllocateDescriptorSets(m_device, &allocInfo, set);
	if (allocResult != VK_SUCCESS) {
		LOG_ERROR("Failed to allocate required descriptor set after needed reallocation. Error: {}", allocResult);
		throw std::runtime_error("Failed to allocate required descriptor set after needed reallocation");
	}
}

void DescriptorAllocator::init(VkDevice device) {
	m_device = device;
}

void DescriptorAllocator::destroy() {
	for (auto pool : m_freePools) {
		vkDestroyDescriptorPool(m_device, pool, nullptr);
	}
	for (auto pool : m_usedPools) {
		vkDestroyDescriptorPool(m_device, pool, nullptr);
	}
}

VkDescriptorPool DescriptorAllocator::grabPool() {
	if (m_freePools.size() > 0) {
		VkDescriptorPool pool = m_freePools.back();
		m_freePools.pop_back();
		return pool;
	}
	return createPool(m_device, m_descriptorSizes, DESCRIPTOR_ALLOCATOR_BATCH_SIZE, 0);
}

void DescriptorLayoutCache::init(VkDevice device) {
	m_device = device;
}

void DescriptorLayoutCache::destroy() {
	for (const auto &[info, layout] : m_layoutCache) {
		vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
	}
}

VkDescriptorSetLayout DescriptorLayoutCache::createDescriptorLayout(VkDescriptorSetLayoutCreateInfo *info) {
	DescriptorLayoutInfo layoutInfo;
	layoutInfo.bindings.reserve(info->bindingCount);

	bool sorted = true;
	int32_t lastBinding = -1;
	for (uint32_t i = 0; i < info->bindingCount; ++i) {
		layoutInfo.bindings.push_back(info->pBindings[i]);

		if (static_cast<int32_t>(info->pBindings[i].binding) > lastBinding) {
			lastBinding = static_cast<int32_t>(info->pBindings[i].binding);
		}
		else {
			sorted = false;
		}
	}
	if (!sorted) {
		std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(), [](VkDescriptorSetLayoutBinding &a, VkDescriptorSetLayoutBinding &b) {
			return a.binding < b.binding;
		});
	}

	auto it = m_layoutCache.find(layoutInfo);
	if (it != m_layoutCache.end()) {
		return (*it).second;
	}

	VkDescriptorSetLayout layout;
	vkCreateDescriptorSetLayout(m_device, info, nullptr, &layout);

	m_layoutCache[layoutInfo] = layout;
	return layout;
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo &other) const {
	if (other.bindings.size() != bindings.size()) {
		return false;
	}

	for (size_t i = 0; i < bindings.size(); ++i) {
		if (other.bindings[i].binding != bindings[i].binding) {
			return false;
		}
		else if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
			return false;
		}
		else if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
			return false;
		}
		else if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
			return false;
		}
	}
	return true;
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
	using std::size_t;
	using std::hash;

	size_t result = hash<size_t>()(bindings.size());

	for (const VkDescriptorSetLayoutBinding &binding : bindings) {
		// Pack the binding data into a single int64. Not fully correct but its OK
		size_t binding_hash = binding.binding | binding.descriptorType << 8 | binding.descriptorCount << 16 | binding.stageFlags << 24;
		// Shuffle the packed binding data and xor it with the main hash
		result ^= hash<size_t>()(binding_hash);
	}
	return result;
}

DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator) {
	DescriptorBuilder builder;
	builder.m_cache = layoutCache;
	builder.m_allocator = allocator;
	return builder;
}

DescriptorBuilder &DescriptorBuilder::bindBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags) {
	VkDescriptorSetLayoutBinding newBindings{};
	newBindings.descriptorCount = 1;
	newBindings.descriptorType = type;
	newBindings.pImmutableSamplers = nullptr;
	newBindings.stageFlags = stageFlags;
	newBindings.binding = binding;
	
	m_bindings.push_back(newBindings);

	VkWriteDescriptorSet newWrite{};
	newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	newWrite.pNext = nullptr;
	newWrite.descriptorCount = 1;
	newWrite.descriptorType = type;
	newWrite.pBufferInfo = bufferInfo;
	newWrite.dstBinding = binding;

	m_writes.push_back(newWrite);
	return *this;
}

DescriptorBuilder &DescriptorBuilder::bindImage(uint32_t binding, VkDescriptorImageInfo *imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags) {
	VkDescriptorSetLayoutBinding newBinding{};
	newBinding.descriptorCount = 1;
	newBinding.descriptorType = type;
	newBinding.pImmutableSamplers = nullptr;
	newBinding.stageFlags = stageFlags;
	newBinding.binding = binding;

	m_bindings.push_back(newBinding);

	VkWriteDescriptorSet newWrite{};
	newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	newWrite.pNext = nullptr;
	newWrite.descriptorCount = 1;
	newWrite.descriptorType = type;
	newWrite.pImageInfo = imageInfo;
	newWrite.dstBinding = binding;

	m_writes.push_back(newWrite);
	return *this;
}

DescriptorBuilder &DescriptorBuilder::bindDummy(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags) {
	VkDescriptorSetLayoutBinding newBinding{};
	newBinding.descriptorCount = 1;
	newBinding.descriptorType = type;
	newBinding.pImmutableSamplers = nullptr;
	newBinding.stageFlags = stageFlags;
	newBinding.binding = binding;

	m_bindings.push_back(newBinding);
	return *this;
}

void DescriptorBuilder::build(VkDescriptorSet &set, VkDescriptorSetLayout &layout) {
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
	layoutInfo.pBindings = m_bindings.data();

	layout = m_cache->createDescriptorLayout(&layoutInfo);

	m_allocator->allocate(&set, layout);

	for (VkWriteDescriptorSet &write : m_writes) {
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(m_allocator->getDevice(), static_cast<uint32_t>(m_writes.size()), m_writes.data(), 0, nullptr);
}

void DescriptorBuilder::build(VkDescriptorSet &set) {
	VkDescriptorSetLayout layout;
	build(set, layout);
}

void DescriptorBuilder::buildLayout(VkDescriptorSetLayout &layout) {
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
	layoutInfo.pBindings = m_bindings.data();

	layout = m_cache->createDescriptorLayout(&layoutInfo);
}

