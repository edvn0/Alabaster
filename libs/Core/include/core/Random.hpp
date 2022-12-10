#pragma once

#include <effolkronium/random.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Alabaster {

	using Random = effolkronium::random_static;

	inline glm::vec3 random_vec3()
	{
		return glm::vec3 { Random::get<float>(-1.0f, 1.0f), Random::get<float>(-1.0f, 1.0f), Random::get<float>(-1.0f, 1.0f) };
	}

	inline glm::vec3 uniform_random_vec3()
	{
		return glm::vec3 { Random::get<std::uniform_real_distribution<>>(), Random::get<std::uniform_real_distribution<>>(),
			Random::get<std::uniform_real_distribution<>>() };
	}

	inline float uniform_float() { return Random::get<std::uniform_real_distribution<>>(); }

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
		const auto u = uniform_float();
		const auto v = uniform_float();
		const auto theta = u * 2.0 * glm::pi<float>();
		const auto phi = glm::acos(2.0 * v - 1.0);
		const auto r = std::cbrt(uniform_float());
		const auto sin_theta = glm::sin(theta);
		const auto cos_theta = glm::cos(theta);
		const auto sin_phi = glm::sin(phi);
		const auto cos_phi = glm::cos(phi);
		const auto x = r * sin_phi * cos_theta;
		const auto y = r * sin_phi * sin_theta;
		const auto z = r * cos_phi;
		return { radius * x, radius * y, radius * z };
	}

} // namespace Alabaster
