#pragma once

#include <effolkronium/random.hpp>
#include <glm/glm.hpp>

namespace Alabaster {

	using Random = effolkronium::random_static;

	inline glm::vec3 random_vec3()
	{
		return glm::vec3 { Random::get<float>(-1.0f, 1.0f), Random::get<float>(-1.0f, 1.0f), Random::get<float>(-1.0f, 1.0f) };
	}

	inline glm::vec3 uniform_random_vec3()
	{
		return glm::vec3 { Random::get<std::normal_distribution<>>(), Random::get<std::normal_distribution<>>(),
			Random::get<std::normal_distribution<>>() };
	}

	inline glm::vec3 uniform_random_vec3(auto&& from, auto&& to)
	{
		return glm::vec3 { Random::get<std::normal_distribution<>>(from, to), Random::get<std::normal_distribution<>>(from, to),
			Random::get<std::normal_distribution<>>(from, to) };
	}

	inline glm::vec4 random_vec4(float min = -0.5f, float max = 0.5f)
	{
		return glm::vec4 { Random::get<float>(min, max), Random::get<float>(min, max), Random::get<float>(min, max), Random::get<float>(min, max) };
	}

	inline glm::vec3 sphere_vector3(float radius)
	{
		auto random_vec = uniform_random_vec3(-radius, radius);
		while (glm::dot(random_vec, random_vec) > radius * radius) {
			random_vec = uniform_random_vec3(-radius, radius);
		}

		glm::vec3 normalised = glm::normalize(random_vec);
		return normalised * radius;
	}

} // namespace Alabaster
