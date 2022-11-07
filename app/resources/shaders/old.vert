#version 460

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 colour;

layout(location = 0) out vec4 out_colour;

void main()
{

	gl_Position = position;
	out_colour = colour;
}
