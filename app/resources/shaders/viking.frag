#version 450

layout(location = 0) in vec4 colour;
layout(location = 1) in vec2 uvs;

layout(location = 0) out vec4 out_colour;

layout(binding = 1) uniform sampler2D texture_sampler;

void main() { out_colour = colour * texture(texture_sampler, uvs); }
