#pragma once


#include "message.hpp"
#include "../networking/ip_addr.hpp"
#include "../networking/sock_addr.hpp"
#include "../utils/overloaded.hpp"

#include <netinet/in.h>


namespace NApi {
    template <>
    struct Message<MessageType::SocketAddress>
        : public std::variant<sockaddr_in, sockaddr_in6>
    {
        using std::variant<sockaddr_in, sockaddr_in6>::variant;
        // We use these constants instead of the standard
        // constants `AF_INET` and `AF_INET6` defined in
        // `<sys/socket.h>` because the values of `AF_INET`
        // and `AF_INET6` may be platform-dependent
        enum class AddrFamily : uint8_t {
            IPv4 = 4,
            IPv6 = 6,
        };
        static constexpr size_t kSerializedSize =
            sizeof(AddrFamily) + sizeof(in_port_t) +
            std::max(
                sizeof(sockaddr_in::sin_addr),
                sizeof(sockaddr_in6::sin6_addr)
            );
        auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept -> void;
        struct UnknownAddressFamily {
            std::underlying_type_t<AddrFamily> Value;
        };
        using DeserializationError = UnknownAddressFamily;
        static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
          -> std::variant<Message, DeserializationError>;
        auto operator==(const Message& other) const -> bool = default;
    };
    using SocketAddressMsg = Message<MessageType::SocketAddress>;

    template <class OStream>
    inline auto operator<<(OStream&& out, const SocketAddressMsg& x) -> OStream&& {
        out << "SocketAddressMessage{";
        std::visit(overloaded{
            [&out](const sockaddr_in& ipV4Addr) {
                out << "IpV4{"
                       ".port = " << ntohs(ipV4Addr.sin_port) << ", "
                    << ".addr = " << ipV4Addr.sin_addr
                    << "}";
            },
            [&out](const sockaddr_in6& ipV6Addr) {
                out << "IpV6{"
                       ".port = " << ntohs(ipV6Addr.sin6_port) << ", "
                    << ".addr = " << ipV6Addr.sin6_addr
                    << "}";
            },
        }, x);
        out << "}";
        return std::forward<OStream>(out);
    }
} // namespace NApi
