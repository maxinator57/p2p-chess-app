#include "message.hpp"


namespace NApi {
    auto ToString(MessageType mt) -> std::string_view {
        using enum MessageType;
        switch (mt) {
            case CreateNewGameRequest:
                return "CreateNewGameRequest";
            case CreateNewGameResponse:
                return "CreateNewGameResponse";
            case JoinGameRequest:
                return "JoinGameRequest";
            case JoinGameResponse:
                return "JoinGameResponse";
            default:
                return "UnknownMessageType";
        }
    }
} // namespace NApi