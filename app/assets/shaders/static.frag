#version 460

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 worldPosition;


layout(location = 0) out vec4 fragColor;


layout(set = 1, binding = 1) uniform sampler2D albedoSampler;
layout(set = 1, binding = 2) uniform sampler2D normalSampler;


void main() {
	vec3 dir = vec3(1, -1, -1);
	float x = dot(dir, normal);

	vec4 color = texture(albedoSampler, uv);
	vec3 normal = texture(normalSampler, uv).rgb;

	fragColor = vec4(color.rgb * normal, 1.0);
}
