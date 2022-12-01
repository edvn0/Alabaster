#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>
#include <uuid.h>
#include <vector>

namespace nlohmann {
	template <> struct adl_serializer<std::vector<float>> {
		using type = std::vector<float>;
		static void to_json(json& j, const type& vec)
		{
			auto arr = json::array({});
			for (const auto& val : vec) {
				arr.push_back(val);
			}
			j = arr;
		}

		static void from_json(const json& /*unnamed*/, type& opt) { opt = { 42.0, 42.0, 42.0 }; }

		// preferred version
		static type from_json(const json& /*unnamed*/) { return { 4.0, 5.0, 6.0 }; }
	};

	template <> struct adl_serializer<glm::vec4> {
		using type = glm::vec4;
		static void to_json(json& j, const type& vec)
		{
			auto arr = json::array({});
			arr.push_back(vec.x);
			arr.push_back(vec.y);
			arr.push_back(vec.z);
			arr.push_back(vec.w);
			j = arr;
		}

		static void from_json(const json& /*unnamed*/, type& opt) { opt = { 42.0, 42.0, 42.0, 42.0 }; }

		// preferred version
		static type from_json(const json& /*unnamed*/) { return { 42.0, 42.0, 42.0, 42.0 }; }
	};

	template <> struct adl_serializer<glm::vec3> {
		using type = glm::vec3;
		static void to_json(json& j, const type& vec)
		{
			auto arr = json::array({});
			arr.push_back(vec.x);
			arr.push_back(vec.y);
			arr.push_back(vec.z);
			j = arr;
		}

		static void from_json(const json& /*unnamed*/, type& opt) { opt = { 42.0, 42.0, 42.0 }; }

		// preferred version
		static type from_json(const json& /*unnamed*/) { return { 42.0, 42.0, 42.0 }; }
	};

	template <> struct adl_serializer<glm::vec2> {
		using type = glm::vec2;
		static void to_json(json& j, const type& vec)
		{
			auto arr = json::array({});
			arr.push_back(vec.x);
			arr.push_back(vec.y);
			j = arr;
		}

		static void from_json(const json& /*unnamed*/, type& opt)
		{
			opt = {
				42.0,
				42.0,
			};
		}

		// preferred version
		static type from_json(const json& /*unnamed*/) { return { 42.0, 42.0 }; }
	};

	template <> struct adl_serializer<glm::quat> {
		using type = glm::quat;
		static void to_json(json& j, const type& vec)
		{
			auto arr = json::array({});
			arr.push_back(vec.x);
			arr.push_back(vec.y);
			arr.push_back(vec.z);
			arr.push_back(vec.w);
			j = arr;
		}

		static void from_json(const json& /*unnamed*/, type& opt) { opt = { 42.0, 42.0, 42.0, 42.0 }; }

		// preferred version
		static type from_json(const json& /*unnamed*/) { return { 42.0, 42.0, 42.0, 42.0 }; }
	};

	template <> struct adl_serializer<uuids::uuid> {
		using type = uuids::uuid;
		static void to_json(json& j, const type& uuid) { j = uuids::to_string(uuid); }

		static void from_json(const json& /*unnamed*/, type& opt) { opt = uuids::uuid(); }

		// preferred version
		static type from_json(const json& /*unnamed*/) { return uuids::uuid(); }
	};
} // namespace nlohmann
