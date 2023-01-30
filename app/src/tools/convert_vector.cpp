#include "convert_vector.h"


glm::vec4 toVec4(const std::vector<double> &value) {
	return glm::vec4(
		static_cast<float>(value[0]),
		static_cast<float>(value[1]),
		static_cast<float>(value[2]),
		static_cast<float>(value[3]));
}

glm::vec3 toVec3(const std::vector<double> &value) {
	return glm::vec3(
		static_cast<float>(value[0]),
		static_cast<float>(value[1]),
		static_cast<float>(value[2]));
}
