#pragma once

#include <effolkronium/random.hpp>
#include <glm/glm.hpp>

namespace Alabaster {

	using Random = effolkronium::random_static;

	inline static glm::vec3 random_vec3()
	{
		return glm::vec3 { Random::get<float>(-1.0f, 1.0f), Random::get<float>(-1.0f, 1.0f), Random::get<float>(-1.0f, 1.0f) };
	}

	inline static glm::vec4 random_vec4(float min = -0.5f, float max = 0.5f)
	{
		return glm::vec4 { Random::get<float>(min, max), Random::get<float>(min, max), Random::get<float>(min, max), Random::get<float>(min, max) };
	}

	inline static glm::vec3 sphere_vector3(float radius)
	{
		double x, y, z;
		x = Random::get<std::normal_distribution<>>();
		y = Random::get<std::normal_distribution<>>();
		z = Random::get<std::normal_distribution<>>();

		glm::vec3 normalised = glm::normalize(glm::vec3 { x, y, z });
		return normalised * radius;
	}

} // namespace Alabaster
