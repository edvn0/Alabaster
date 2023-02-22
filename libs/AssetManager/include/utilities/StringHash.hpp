#include <string_view>

namespace AssetManager {

	struct StringHash {
		using is_transparent = void;

		std::size_t operator()(std::string_view sv) const
		{
			std::hash<std::string_view> hasher;
			return hasher(sv);
		}
	};

} // namespace AssetManager
