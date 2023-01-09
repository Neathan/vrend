#pragma once

#include <string>
#include <cstdint>

struct AppInfo {
	std::string name;

	uint32_t versionVariant;
	uint32_t versionMajor;
	uint32_t versionMinor;
	uint32_t versionPatch;
};
