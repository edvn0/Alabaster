#version 460
layout(location = 0) in vec3 locations;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in vec2 uvs;

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

layout(location = 0) out vec4 out_colour;
layout(location = 1) out vec2 out_uvs;
layout(location = 2) out vec3 out_normals;
layout(location = 3) out vec4 out_position;

void main()
{
	mat4 model_view = ubo.view * pc.object_transform;
	gl_Position = ubo.view_proj * pc.object_transform * vec4(locations, 1.0);
	out_colour = pc.object_colour;
	out_uvs = uvs;
	out_normals = vec3(model_view * vec4(normal, 0.0));
	out_position = model_view * vec4(locations, 1.0);
}
