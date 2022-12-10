#version 450

layout(push_constant) uniform Renderer3D
{
	vec4 light_position;
	vec4 light_colour;
	vec4 light_ambience;
	vec4 object_colour;
	mat4 object_transform;
}
pc;

layout(location = 0) in vec4 in_colour;
layout(location = 1) in vec2 in_uvs;
layout(location = 2) in vec3 in_normals;
layout(location = 3) in vec4 in_position;

layout(location = 0) out vec4 out_colour;

void main()
{
	float distance = length(pc.light_position - in_position);

	// Get a lighting direction vector from the light to the vertex.
	vec3 lightVector = normalize(vec3(pc.light_position - in_position));

	// Calculate the dot product of the light vector and vertex normal. If the normal and light vector are
	// pointing in the same direction then it will get max illumination.
	float diffuse = max(dot(in_normals, lightVector), 0.1);

	// Add attenuation.
	diffuse = diffuse * (1.0 / (1.0 + (0.25 * distance * distance)));

	// Multiply the color by the diffuse illumination level to get final output color.
	out_colour = in_colour;
}
