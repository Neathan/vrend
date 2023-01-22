#version 460

layout(binding = 0) uniform UniformData {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTextureCoordinate;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 testColor;

void main() {
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	testColor = inNormal;
}
