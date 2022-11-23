#version 460

layout(location = 0) in vec3 locations;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in vec2 uvs;

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
layout(location = 1) out vec2 out_uvs;
layout(location = 2) out vec3 out_normal;
layout(location = 3) out vec3 out_frag_position;

void main()
{
	gl_Position = ubo.view_proj * pc.object_transform * vec4(locations, 1.0);
	out_colour = pc.object_colour;
	out_uvs = uvs;
	out_normal = normal;
}
