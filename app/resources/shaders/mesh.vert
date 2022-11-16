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
	mat4 transform;
	vec4 mesh_colour;
}
pc;

layout(location = 0) out vec4 out_colour;
layout(location = 1) out vec2 out_uvs;

void main()
{
	gl_Position = ubo.view_proj * pc.transform * vec4(locations, 1.0);
	vec4 col_out = pc.mesh_colour * abs(vec4(normal, 1.0));
	out_colour = normalize(col_out);
	out_uvs = uvs;
}
