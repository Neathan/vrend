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

glm::mat4 toMat4(const std::vector<double> &value) {
	return glm::mat4(
		static_cast<float>(value[0]),
		static_cast<float>(value[1]),
		static_cast<float>(value[2]),
		static_cast<float>(value[3]),
		static_cast<float>(value[4]),
		static_cast<float>(value[5]),
		static_cast<float>(value[6]),
		static_cast<float>(value[7]),
		static_cast<float>(value[8]),
		static_cast<float>(value[9]),
		static_cast<float>(value[10]),
		static_cast<float>(value[11]),
		static_cast<float>(value[12]),
		static_cast<float>(value[13]),
		static_cast<float>(value[14]),
		static_cast<float>(value[15])
	);
}
