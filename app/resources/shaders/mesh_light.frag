#version 450

layout(location = 0) in vec4 colour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 position;

layout(location = 0) out vec4 out_colour;

struct PointLight {
	vec4 position;
	vec4 ambience;
};

layout(binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 view_proj;
	vec4 num_lights;
	PointLight point_lights[10];
}
ubo;

layout(push_constant) uniform PC
{
	vec4 light_position;
	vec4 light_colour;
	vec4 light_ambience;
	vec4 object_colour;
	mat4 object_transform;
}
pc;

void main()
{
	vec3 diffuse_light_total = pc.light_ambience.xyz * pc.light_ambience.w;
	vec3 surface_normal = normal;

	int num_lights = int(ubo.num_lights.x);
	for (int i = 0; i < num_lights; i++) {
		PointLight light = ubo.point_lights[i];
		vec3 direction_to_light = light.position.xyz - position;
		float attenuation = 1.0 / dot(direction_to_light, direction_to_light);
		float ang_incidence = max(dot(surface_normal, normalize(direction_to_light)), 0);
		vec3 intensity = light.ambience.xyz * light.ambience.w * attenuation;

		diffuse_light_total += intensity * ang_incidence;
	}

	out_colour = vec4(diffuse_light_total * vec3(colour), 1.0);
}
