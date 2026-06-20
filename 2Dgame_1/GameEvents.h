#pragma once
#include <type_traits>

enum class DeathCause {
	UNKNOWN = 0,
	FALL,
	FALL_HIGH,
	SPIKE,
	SPIKE_RISING,
	TRAP,
	ENEMY_WALKER,
	ENEMY_FLYER,
	ENEMY_SHOOTER,
	OJISAN_PUNCH,
};

// unordered_map Ç≈ enum class ÇÉLÅ[Ç…égÇ§ÇΩÇﬂÇÃ std::hash ì¡éÍâª
namespace std {
    template<>
    struct hash<DeathCause> {
        std::size_t operator()(DeathCause c) const noexcept {
            using UT = std::underlying_type_t<DeathCause>;
            return std::hash<UT>()(static_cast<UT>(c));
        }
    };
}