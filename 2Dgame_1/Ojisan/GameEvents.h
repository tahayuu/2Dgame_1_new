#pragma once
#include <type_traits>

// ================================================================
// GameEvents.h の役割
// ---------------------------------------------------------------
// ・死亡原因など、システム横断で共有するイベント種別を定義する。
// ・DeathCause は台詞分岐や演出分岐のキーとして使う。
// ================================================================

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

// unordered_map で enum class をキーに使うための std::hash 特殊化
namespace std {
    template<>
    struct hash<DeathCause> {
        std::size_t operator()(DeathCause c) const noexcept {
            using UT = std::underlying_type_t<DeathCause>;
            return std::hash<UT>()(static_cast<UT>(c));
        }
    };
}