#version 460

layout(location = 0) in vec3 locations;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uvs;
layout(location = 4) in int texture_index;

layout(binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 view_proj;
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

layout(location = 0) out vec4 out_colour;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_frag_position;
layout(location = 3) out vec2 out_uvs;
layout(location = 4) flat out int out_texture_index;

void main()
{
	vec4 loc = ubo.view_proj * vec4(locations, 1.0);
	gl_Position = loc;
	out_frag_position = locations;

	out_colour = colour;
	out_normal = normal;
	out_uvs = uvs;
	out_texture_index = texture_index;
}
