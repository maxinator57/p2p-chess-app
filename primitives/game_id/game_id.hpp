#pragma once


#include <compare>
#include <cstdint>


class GameId {
private:
    using ValueType = uint64_t;
    ValueType Value;
public:
    GameId(ValueType value) : Value(value) {}
    auto operator<=>(const GameId& other) const noexcept
      -> std::strong_ordering = default;
    static auto CreateRandom() noexcept -> GameId;
};
