#include "core/UUID.hpp"

#include "core/Random.hpp"

namespace Alabaster::UUID {

	static uuids::basic_uuid_random_generator uuid_generator(Alabaster::Random::engine());

	uuids::uuid random_uuid() { return uuid_generator(); }

} // namespace Alabaster::UUID
