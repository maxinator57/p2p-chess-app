#pragma once


#include "../../utils/error.hpp"

#include <compare>
#include <cstdint>
#include <span>
#include <string_view>
#include <variant>


class GameId {
private:
    using ValueType = uint64_t;
    ValueType Value;
public:
    GameId(ValueType value) : Value(value) {}
    auto operator<=>(const GameId& other) const noexcept
      -> std::strong_ordering = default;
    static auto CreateRandom() noexcept -> GameId;
    auto GetValue() const -> ValueType;
    auto ToString() const -> std::string;
    static auto FromString(std::string_view) noexcept
      -> std::variant<GameId, SystemError>;
    static constexpr auto kSerializedSize = sizeof(Value);
    auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept -> void; 
    static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
      -> GameId;
};

template <class OStream>
auto operator<<(OStream&& out, const GameId& gameId) -> OStream&& {
    out << "GameId{" << gameId.GetValue() << "}";
    return std::forward<OStream>(out);
}
