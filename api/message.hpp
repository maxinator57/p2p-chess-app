#pragma once

#include "../utils/integer_serialization.hpp"
#include "../utils/span_utils.hpp"

#include <cstdint>
#include <span>
#include <string_view>
#include <variant>


namespace NApi {
    /*
     * Warning: the values of the following constants
     * must be always specified and must never be changed,
     * because these actions can potentially break backward
     * and forward compatibility 
     */ 
    enum class MessageType : uint8_t {
        CreateNewGameRequest = 1,
        CreateNewGameResponse = 2,
        JoinGameRequest = 3,
        JoinGameResponse = 4,
        SocketAddress = 5,
    };
    constexpr auto IsKnownMessageType(MessageType mt) noexcept -> bool {
        using enum MessageType;
        switch (mt) {
            case CreateNewGameRequest:  [[fallthrough]];
            case CreateNewGameResponse: [[fallthrough]];
            case JoinGameRequest:       [[fallthrough]];
            case JoinGameResponse:      [[fallthrough]];
            case SocketAddress:
                return true;
            default:
                return false;
        }
    }
    constexpr auto ToString(MessageType mt) noexcept -> std::string_view {
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
            case SocketAddress:
                return "SocketAddress";
            default:
                return "UnknownMessageType";
        }
    }
    template <class OStream>
    inline auto operator<<(OStream&& out, MessageType mt) noexcept -> OStream&& {
        out << ToString(mt);
        return std::forward<OStream>(out);
    }

    struct UnknownMessageTypeError {
        std::underlying_type_t<MessageType> UnknownMessageType;
    };
    template <class OStream>
    inline auto operator<<(OStream&& out, UnknownMessageTypeError err) noexcept -> OStream&& {
        out << "unknown message type: " << err.UnknownMessageType;
        return std::forward<OStream>(out);
    }

    struct WrongMessageTypeError {
        MessageType ExpectedMessageType;
        MessageType GotMessageType;
    };
    template <class OStream>
    inline auto operator<<(OStream&& out, WrongMessageTypeError err) noexcept -> OStream&& {
        out << "wrong message type: expected " << err.ExpectedMessageType << ", got " << err.GotMessageType;
        return std::forward<OStream>(out);
    }

    template <MessageType MT> struct Message;

    template <MessageType MT>
    using Buf = std::array<std::byte, sizeof(MT) + Message<MT>::kSerializedSize>;
    
    template <MessageType MT>
    using View = std::span<std::byte, sizeof(MT) + Message<MT>::kSerializedSize>;

    template <MessageType MT>
    using ConstView = std::span<const std::byte, sizeof(MT) + Message<MT>::kSerializedSize>;

    template <MessageType MT> concept IsValidMessage =
        (Message<MT>::kSerializedSize == 0) || requires(Message<MT> msg) {
            { []() { return Message<MT>::kSerializedSize; }() } -> std::same_as<size_t>;
            {
                msg.ToBytes(
                    std::declval<std::span<std::byte, Message<MT>::kSerializedSize>>()
                ) 
            } -> std::same_as<void>;
            {
                Message<MT>::FromBytes(
                    std::declval<std::span<const std::byte, Message<MT>::kSerializedSize>>()
                )
            };
            { msg == std::declval<Message<MT>>() } -> std::same_as<bool>;
        }; 

    template <MessageType MT>
    inline auto Serialize(const Message<MT>& msg, View<MT> to) noexcept -> void {
        static_assert(IsValidMessage<MT>);
        if constexpr (Message<MT>::kSerializedSize == 0) {
            EnumToBytes(MT, to);
        } else {
            auto [lSpan, rSpan] = Split<sizeof(MT)>(to);
            EnumToBytes(MT, lSpan);
            msg.ToBytes(rSpan);
        }
    }

    template <MessageType MT>
    inline auto Serialize(const Message<MT>& msg) noexcept -> Buf<MT> {
        Buf<MT> to;
        Serialize(msg, to);
        return to;
    }

    template <MessageType MT> concept IsDeserializableWithoutErrors = requires {
        {
            Message<MT>::FromBytes(
                std::declval<std::span<const std::byte, Message<MT>::kSerializedSize>>()
            )
        } -> std::same_as<Message<MT>>;
    };

    template <MessageType MT> concept IsDeserializableWithErrors = requires {
        {
            Message<MT>::FromBytes(
                std::declval<std::span<const std::byte, Message<MT>::kSerializedSize>>()
            )
        } -> std::same_as<std::variant<Message<MT>, typename Message<MT>::DeserializationError>>;
    };

    // Deserialization function for messages that
    // have deserialization zero serialized size
    template <MessageType MT>
    inline auto Deserialize(ConstView<MT> from) noexcept
      -> std::variant<Message<MT>, UnknownMessageTypeError, WrongMessageTypeError>
    requires (Message<MT>::kSerializedSize == 0)
    {
        const auto actualMessageType = EnumFromBytes<MessageType>(from);
        if (actualMessageType == MT) {
            return Message<MT>();
        } else if (IsKnownMessageType(actualMessageType)) {
            return WrongMessageTypeError{
                .ExpectedMessageType = MT,
                .GotMessageType = actualMessageType,
            };
        } else {
            return UnknownMessageTypeError{
                .UnknownMessageType = ToUnderlying(actualMessageType),
            };
        }
    }

    // Deserialization function for messages that
    // don't have deserialization errors
    template <MessageType MT>
    inline auto Deserialize(ConstView<MT> from) noexcept
      -> std::variant<Message<MT>, UnknownMessageTypeError, WrongMessageTypeError>
    requires IsDeserializableWithoutErrors<MT>
    {
        static_assert(IsValidMessage<MT>);
        const auto [lSpan, rSpan] = Split<sizeof(MT)>(from);
        const auto actualMessageType = EnumFromBytes<MessageType>(lSpan);
        if (actualMessageType == MT) {
            return Message<MT>::FromBytes(rSpan);
        } else if (IsKnownMessageType(actualMessageType)) {
            return WrongMessageTypeError{
                .ExpectedMessageType = MT,
                .GotMessageType = actualMessageType,
            };
        } else {
            return UnknownMessageTypeError{
                .UnknownMessageType = ToUnderlying(actualMessageType),
            };
        }
    }

    // Deserialization function for messages that
    // can have deserialization errors and therefore
    // define a `DeserializationError` member type
    template <MessageType MT>
    requires IsDeserializableWithErrors<MT>
    inline auto Deserialize(ConstView<MT> from) noexcept
      -> std::variant<Message<MT>, typename Message<MT>::DeserializationError,
                      UnknownMessageTypeError, WrongMessageTypeError>
    requires IsDeserializableWithErrors<MT>
    {
        static_assert(IsValidMessage<MT>);
        using ReturnType =
            std::variant<Message<MT>, typename Message<MT>::DeserializationError,
                         UnknownMessageTypeError, WrongMessageTypeError>;
        const auto [lSpan, rSpan] = Split<sizeof(MT)>(from);
        const auto actualMessageType = EnumFromBytes<MessageType>(lSpan);
        if (actualMessageType == MT) {
            auto msgOrErr = Message<MT>::FromBytes(rSpan);
            return std::visit(
                [](auto&& msgOrErr) -> ReturnType { return std::move(msgOrErr); },
                std::move(msgOrErr)
            );
        } else if (IsKnownMessageType(actualMessageType)) {
            return WrongMessageTypeError{
                .ExpectedMessageType = MT,
                .GotMessageType = actualMessageType,
            };
        } else {
            return UnknownMessageTypeError{
                .UnknownMessageType = ToUnderlying(actualMessageType),
            };
        }
    }
    
} // namespace NApi
