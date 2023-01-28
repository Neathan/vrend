#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 worldPosition;


layout(binding = 0) uniform UniformData {
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform TransformData {
	mat4 matrix;
} transform;


void main() {
	gl_Position = ubo.proj * ubo.view * transform.matrix * vec4(inPosition, 1.0);
	normal = mat3(transform.matrix) * inNormal;
	uv = inTextureCoordinate;
}
