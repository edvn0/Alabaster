#version 450

layout(location = 0) in vec4 colour;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 position;
layout(location = 3) in vec2 uvs;

layout(location = 0) out vec4 out_colour;

layout(push_constant) uniform PC
{
	vec4 light_position;
	vec4 light_colour;
	vec4 light_ambience;
	vec4 object_colour;
	mat4 object_transform;
}
pc;

void main()
{
	vec3 light_pos = vec3(pc.light_position);
	vec3 light_colour = vec3(pc.light_colour);
	float ambient_strength = pc.light_ambience.x;
	vec3 ambient = ambient_strength * light_colour;

	vec3 norm = normalize(normal);
	vec3 light_dir = normalize(light_pos - vec3(position));
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diff * light_colour;

	vec3 result = (ambient + diffuse) * vec3(colour);
	out_colour = vec4(result, 1.0);
}
