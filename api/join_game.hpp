#pragma once


#include "message.hpp"

#include "../primitives/game_id/game_id.hpp"
#include "../primitives/player_id/player_id.hpp"

#include <type_traits>


namespace NApi {
    struct AddPlayerToGameOp {
        PlayerId Player;
        GameId Game;
        enum class Result : uint8_t {
            Success = 40,
            GameIdDoesNotExist = 41,
            GameAlreadyHasTwoPlayers = 42,
        };
    }; 

    template <>
    struct Message<MessageType::JoinGameRequest> {
        using Self = Message<MessageType::JoinGameRequest>;
        GameId GameIdToJoin;
        static constexpr auto kSerializedSize = sizeof(GameIdToJoin);
        auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept -> void;
        static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
          -> Self;
        // TODO: get rid of this dummy struct
        struct ParsingError {};
    };
    using JoinGameRequest = Message<MessageType::JoinGameRequest>;

    template <>
    struct Message<MessageType::JoinGameResponse> {
        using Self = Message<MessageType::JoinGameResponse>;
        AddPlayerToGameOp::Result Result;
        static constexpr auto kSerializedSize = sizeof(Result);
        auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept -> void;
        struct UnknownResultError {
            std::underlying_type_t<AddPlayerToGameOp::Result> Value;
        };
        static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
          -> std::variant<Self, UnknownResultError>;
        using ParsingError = UnknownResultError;
    };
    using JoinGameResponse = Message<MessageType::JoinGameResponse>;
} // namespace NApi
