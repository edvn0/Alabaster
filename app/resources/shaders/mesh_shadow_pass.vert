#version 460

layout(location = 0) in vec3 locations;

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

layout(push_constant) uniform Renderer3D
{
	vec4 light_position;
	vec4 light_colour;
	vec4 light_ambience;
	vec4 object_colour;
	mat4 object_transform;
}
pc;

void main() { gl_Position = ubo.view_proj * pc.object_transform * vec4(locations, 1.0); }