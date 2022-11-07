#version 460

layout(location = 0) in vec4 locations;
layout(location = 1) in vec4 colour;

layout(binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 view_proj;
}
ubo;

layout(location = 0) out vec4 out_colour;

void main()
{
	gl_Position = ubo.view_proj * locations;
	out_colour = colour;
}
