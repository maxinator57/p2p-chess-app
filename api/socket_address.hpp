#pragma once


#include "message.hpp"

#include <netinet/in.h>


namespace NApi {
    template <>
    struct Message<MessageType::SocketAddress> : public std::variant<sockaddr_in, sockaddr_in6> {
        using std::variant<sockaddr_in, sockaddr_in6>::variant;
        enum class AddrFamily : uint8_t {
            IPv4 = 4,
            IPv6 = 6,
        };
        static constexpr auto kSerializedSize = sizeof(AddrFamily) + std::max(
            sizeof(sockaddr_in::sin_port) + sizeof(sockaddr_in::sin_addr),
            sizeof(sockaddr_in6::sin6_port) + sizeof(sockaddr_in6::sin6_addr)
        );
        auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept -> void;
        struct UnknownAddressFamily {
            std::underlying_type_t<AddrFamily> Value;
        };
        using DeserializationError = UnknownAddressFamily;
        static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
          -> std::variant<Message, DeserializationError>;
    };
    using SocketAddressMsg = Message<MessageType::SocketAddress>;
} // namespace NApi
