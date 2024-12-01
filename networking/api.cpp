#include "api.hpp"

#include "../utils/integer_serialization.hpp"
#include "../utils/overloaded.hpp"


namespace NApi {
    auto Request::ToBytes(std::span<std::byte, kSerializedSize> to) const noexcept
      -> void {
        ::ToBytes(Type, to);
    }
    auto Request::FromBytes(std::span<const std::byte, kSerializedSize> from) noexcept
      -> std::variant<Request, UnknownRequestTypeError> {
        const auto requestType = ::FromBytes<RequestType>(from);
        switch (requestType) {
            case RequestType::CreateNewGame: [[fallthrough]];
            case RequestType::JoinGame:
                return Request{.Type = requestType};
            default:
                return UnknownRequestTypeError{
                    .UnknownType = static_cast<uint8_t>(requestType)
                };
        }
    }

    CreateNewGameRequest::CreateNewGameRequest() noexcept
        : Request{.Type = RequestType::CreateNewGame}
    {
    } 

    JoinGameRequest::JoinGameRequest(GameId&& gameId) noexcept
        : Request{.Type = RequestType::JoinGame}
        , Id(std::move(gameId))
    {
    }
    auto JoinGameRequest::ToBytes(std::span<std::byte, kSerializedSize> to) const noexcept
      -> void {
        Request::ToBytes(to.subspan<0, 1>());
        Id.ToBytes(to.subspan<1, kSerializedSize - 1>());
    }
    auto JoinGameRequest::FromBytes(std::span<const std::byte, kSerializedSize> from) noexcept
      -> std::variant<JoinGameRequest, UnknownRequestTypeError, WrongRequestTypeError> {
        using R = std::variant<JoinGameRequest, UnknownRequestTypeError, WrongRequestTypeError>;
        const auto requestTypeOrErr = Request::FromBytes(from.subspan<0, 1>());
        return std::visit(overloaded{
            [&from](Request request) -> R {
                if (request.Type == RequestType::JoinGame) {
                    return JoinGameRequest(
                        GameId::FromBytes(from.subspan<1, kSerializedSize - 1>())
                    );
                } else {
                    return WrongRequestTypeError{.WrongType = request.Type};
                }
            },
            [](auto err) -> R { return err; }
        }, requestTypeOrErr);
    }
} // namespace NApi
