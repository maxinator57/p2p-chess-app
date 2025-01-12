#pragma once


#include <cstdint>
#include <string_view>
#include <type_traits>


namespace NApi {
    enum class MessageType : uint8_t {
        CreateNewGameRequest = 70,
        CreateNewGameResponse,
        JoinGameRequest,
        JoinGameResponse,
    };
    auto ToString(MessageType) -> std::string_view;
    template <class OStream>
    inline auto operator<<(OStream& out, MessageType mt) -> OStream& {
        out << ToString(mt);
        return out;
    }

    struct UnknownMessageTypeError {
        std::underlying_type_t<MessageType> UnknownMessageType;
    };
    template <class OStream>
    inline auto operator<<(OStream& out, UnknownMessageTypeError err) -> OStream& {
        out << "unknown message type: " << err.UnknownMessageType;
        return out;
    }

    struct WrongMessageTypeError {
        MessageType ExpectedMessageType;
        MessageType GotMessageType;
    };
    template <class OStream>
    inline auto operator<<(OStream& out, WrongMessageTypeError err) -> OStream& {
        out << "wrong message type: expected " << err.ExpectedMessageType << ", got " << err.GotMessageType;
        return out;
    }

    template <MessageType MT> struct Message; 
} // namespace NApi
