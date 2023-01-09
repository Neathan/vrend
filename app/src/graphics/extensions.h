#pragma once

#include <glad/vulkan.h>

#include <vector>


class Extensions {
public:
	void addGLFW();
	void add(const char *extension) {
		m_extensions.push_back(extension);
	}
	void add(const std::initializer_list<const char *> &extensions) {
		m_extensions.insert(m_extensions.end(), extensions);
	}

	bool deviceCompatible(VkPhysicalDevice device) const;

	const std::vector<const char *> &getExtensions() const { return m_extensions; }

private:
	std::vector<const char *> m_extensions;
};
