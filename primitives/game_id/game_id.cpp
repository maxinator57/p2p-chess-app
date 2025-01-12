#include "game_id.hpp"

#include "../../utils/integer_serialization.hpp"

#include <charconv>
#include <climits>
#include <random>


auto GameId::CreateRandom() noexcept -> GameId {
    static constexpr auto kRngSeed = 57;
    static auto rng = std::mt19937_64{kRngSeed};
    return GameId(rng());
}

auto GameId::GetValue() const -> ValueType {
    return Value;
}
auto GameId::ToString() const -> std::string {
    return std::to_string(Value);
}

auto GameId::FromString(std::string_view str) noexcept
  -> std::variant<GameId, SystemError> {
    ValueType value;
    const auto [_, ec] = std::from_chars(
        str.data(),
        str.data() + str.size(),
        value
    );
    if (ec == std::errc{}) {
        return GameId{value};
    } else {
        return SystemError{.Value = ec};
    }
}

auto GameId::ToBytes(std::span<std::byte, kSerializedSize> to) const noexcept
  -> void {
    IntToBytes(Value, to);
}

auto GameId::FromBytes(std::span<const std::byte, kSerializedSize> from) noexcept
  -> GameId {
    return GameId{IntFromBytes<ValueType>(from)};
}
