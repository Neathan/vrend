#pragma once

#include <vector>

class Validator {
public:
	void add(const std::initializer_list<char *> &layers) { m_layers.insert(m_layers.end(), layers); }
	void clear() { m_layers.clear(); }

	bool valid();
	const std::vector<char *> &getLayers() const { return m_layers; }

private:
	std::vector<char *> m_layers;
};
