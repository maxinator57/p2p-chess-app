#include "join_game.hpp"

#include "../utils/integer_serialization.hpp"


namespace NApi {
    auto JoinGameRequest::ToBytes(std::span<std::byte, kSerializedSize> to) const noexcept
      -> void {
        GameIdToJoin.ToBytes(to);
    }

    auto JoinGameRequest::FromBytes(std::span<const std::byte, kSerializedSize> from) noexcept
      -> JoinGameRequest {
        return JoinGameRequest{
            .GameIdToJoin = GameId::FromBytes(from)
        };
    }

    auto JoinGameResponse::ToBytes(std::span<std::byte, kSerializedSize> to) const noexcept
      -> void {
        EnumToBytes(Result, to);
    }

    auto JoinGameResponse::FromBytes(std::span<const std::byte, kSerializedSize> from) noexcept
      -> std::variant<Self, UnknownResultError> {
        using Result = AddPlayerToGameOp::Result;
        const auto r = EnumFromBytes<Result>(from);
        switch (r) {
            case Result::Success:                  [[fallthrough]];
            case Result::GameAlreadyHasTwoPlayers: [[fallthrough]];
            case Result::GameIdDoesNotExist:
                return JoinGameResponse{.Result = r};
            default:
                return UnknownResultError{static_cast<std::underlying_type_t<Result>>(r)};
        }
    }
} // namespace NApi
