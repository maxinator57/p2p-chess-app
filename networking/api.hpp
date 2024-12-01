#pragma once


#include "../actions/add_player_to_game.hpp"
#include "../actions/create_new_game.hpp"
#include "../primitives/game_id/game_id.hpp"

#include <cstdint>
#include <span>


namespace NApi {
    enum class RequestType : uint8_t {
        CreateNewGame,
        JoinGame,
    };
    struct Request {
        RequestType Type;
        static constexpr auto kSerializedSize = sizeof(Type);
        auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept -> void;
        struct UnknownRequestTypeError {
            uint8_t UnknownType;
        };
        static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
          -> std::variant<Request, UnknownRequestTypeError>;
    };

    struct CreateNewGameRequest : public Request {
    public:
        CreateNewGameRequest() noexcept;
    };
    struct CreateNewGameResponse {
        NActions::CreateNewGameResult Result;
    };

    struct JoinGameRequest : public Request {
        GameId Id;
        explicit JoinGameRequest(GameId&& gameId) noexcept; 
        static constexpr auto kSerializedSize = sizeof(Request) + sizeof(Id);
        auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept -> void;
        struct WrongRequestTypeError {
            RequestType WrongType;
        };
        static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
          -> std::variant<JoinGameRequest, UnknownRequestTypeError, WrongRequestTypeError>;
    };
    struct JoinGameResponse {
        NActions::AddPlayerToGameResult Result;
    };
} // namespace NApi