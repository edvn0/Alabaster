
namespace Alabaster::BitCast {
	template <typename T> static constexpr auto reinterpret_as(auto in) { return reinterpret_cast<T>(in); }
} // namespace Alabaster::BitCast
