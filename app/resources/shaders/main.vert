#version 460

layout(location = 0) in vec4 locations;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 uvs;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 view_proj;
} ubo;

layout(location = 0) out vec4 out_colour;
layout(location = 1) out vec2 out_uvs;

void main() {
	gl_Position = ubo.view_proj * locations;
	out_colour = colour;
	out_uvs = uvs;
}
