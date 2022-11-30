#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <spdlog/fmt/bundled/format.h>

template <> struct fmt::formatter<glm::vec3> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

	template <typename FormatContext> auto format(const glm::vec3& input, FormatContext& ctx) -> decltype(ctx.out())
	{
		return format_to(ctx.out(), "(x={}, y={}, z={})", input.x, input.y, input.z);
	}
};

template <> struct fmt::formatter<glm::vec4> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

	template <typename FormatContext> auto format(const glm::vec4& input, FormatContext& ctx) -> decltype(ctx.out())
	{
		return format_to(ctx.out(), "(x={}, y={}, z={}, w={})", input.x, input.y, input.z, input.w);
	}
};

template <> struct fmt::formatter<glm::vec2> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

	template <typename FormatContext> auto format(const glm::vec2& input, FormatContext& ctx) -> decltype(ctx.out())
	{
		return format_to(ctx.out(), "(x={}, y={})", input.x, input.y);
	}
};

template <> struct fmt::formatter<glm::quat> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

	template <typename FormatContext> auto format(const glm::quat& input, FormatContext& ctx) -> decltype(ctx.out())
	{
		return format_to(ctx.out(), "(x={}, y={}, z={}, w={})", input.x, input.y, input.z, input.w);
	}
};
