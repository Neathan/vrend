#version 460
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 worldPosition;


layout(location = 0) out vec4 fragColor;


layout(set = 1, binding = 1) uniform sampler2D colorSampler;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(set = 1, binding = 3) uniform sampler2D normalSampler;
layout(set = 1, binding = 4) uniform sampler2D occlusionSampler;
layout(set = 1, binding = 5) uniform sampler2D emissiveSampler;

layout(set = 1, binding = 6) uniform MaterialData {
	vec4 colorFactor;
	float metallicFactor;
	float roughnessFactor;
	vec4 emissiveFactor;
	bool dubbleSided;
} material;

void main() {
	vec3 dir = vec3(1, -1, -1);
	float x = dot(dir, normal);

	vec4 color = texture(colorSampler, uv);
	vec2 metallicRoughness = texture(metallicRoughnessSampler, uv).rg;
	vec3 normal = texture(normalSampler, uv).rgb;
	float occlusion = texture(occlusionSampler, uv).r;
	vec3 emissive = texture(emissiveSampler, uv).rgb;

	fragColor = vec4(color.rgb * material.colorFactor.rgb * occlusion, color.a);
}
