#include "socket_address.hpp"

#include "../utils/byte_utils.hpp"
#include "../utils/overloaded.hpp"
#include "../utils/span_utils.hpp"
#include <netinet/in.h>


using NByteUtils::operator""_b;


namespace NApi {
    auto Message<MessageType::SocketAddress>::ToBytes(
        std::span<std::byte, kSerializedSize> to
    ) const noexcept -> void {
        std::visit(overloaded{
            [&to](const sockaddr_in& ipv4Addr) {
                const auto [afSpan, portSpan, ipAddrSpan, rest] =
                    Split<sizeof(AddrFamily), sizeof(sockaddr_in::sin_port), sizeof(sockaddr_in::sin_addr)>(to);
                EnumToBytes(AddrFamily::IPv4, afSpan);
                IntToBytes(ipv4Addr.sin_port, portSpan);
                IntToBytes(ipv4Addr.sin_addr.s_addr, ipAddrSpan);
                std::fill(rest.begin(), rest.end(), 0_b);
            },
            [&to](const sockaddr_in6& ipv6Addr) {
                const auto [afSpan, portSpan, ipAddrSpan] =
                    Split<sizeof(AddrFamily), sizeof(sockaddr_in6::sin6_port)>(to);
                EnumToBytes(AddrFamily::IPv6, afSpan);
                IntToBytes(ipv6Addr.sin6_port, portSpan);
                SpanDeepCopy(SpanDeepCopyArgs{
                    .Src = std::as_bytes(std::span{ipv6Addr.sin6_addr.s6_addr}),
                    .Dst = ipAddrSpan,
                });
            },
        }, *this);
    }

    auto Message<MessageType::SocketAddress>::FromBytes(
        std::span<const std::byte, kSerializedSize> from
    ) noexcept -> std::variant<Message, DeserializationError> {
        const auto [afSpan, rest] = Split<sizeof(AddrFamily)>(from);
        const auto addressFamily = EnumFromBytes<AddrFamily>(afSpan);
        switch (addressFamily) {
            case AddrFamily::IPv4: {
                const auto [portSpan, addrSpan, _] =
                    Split<sizeof(in_port_t), sizeof(sockaddr_in::sin_addr)>(rest);
                auto msg = Message{std::in_place_type_t<sockaddr_in>{}};
                auto& addr = std::get<sockaddr_in>(msg);
                addr.sin_family = AF_INET;
                addr.sin_port = IntFromBytes<in_port_t>(portSpan);
                addr.sin_addr = in_addr{
                    .s_addr = IntFromBytes<in_addr_t>(addrSpan),
                };
                return msg;
            }
            case AddrFamily::IPv6: {
                const auto [portSpan, addrSpan] = Split<sizeof(in_port_t)>(rest);
                auto msg = Message{std::in_place_type_t<sockaddr_in6>{}};
                auto& addr = std::get<sockaddr_in6>(msg);
                addr.sin6_family = AF_INET6;
                addr.sin6_port = IntFromBytes<in_port_t>(portSpan);
                SpanDeepCopy(SpanDeepCopyArgs{
                    .Src = addrSpan,
                    .Dst = std::as_writable_bytes(std::span{addr.sin6_addr.s6_addr})
                });
                return msg;
            }
            default:
                return UnknownAddressFamily{ToUnderlying(addressFamily)};
        }
    }
} // namespace NApi