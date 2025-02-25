#pragma once

#include "../utils/span_utils.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>

#include <ranges>
#include <string_view>
#include <sys/socket.h>
#include <variant>


struct IP {
    struct v4 {
        struct loopback {};
        struct any {};
    };
    struct v6 {
        struct loopback {};
        struct any {};
    };
};

using IpAddr = std::variant<
    std::string_view,
    IP::v4::any,
    IP::v4::loopback,
    IP::v6::any,
    IP::v6::loopback
>;

template <class IpAddrType> constexpr auto AddressFamily();
template <> constexpr auto AddressFamily<IP::v4>() { return AF_INET; }
template <> constexpr auto AddressFamily<IP::v6>() { return AF_INET6; }


template <class IpAddrType> struct IpAddrStorage;
template <> struct IpAddrStorage<IP::v4> { using type = in_addr; };
template <> struct IpAddrStorage<IP::v6> { using type = in6_addr; };
template <class IpAddrType> using ip_addr_storage_t = typename IpAddrStorage<IpAddrType>::type;

template <class IpAddrStorage> struct IpAddrType;
template <> struct IpAddrType<in_addr> { using type = IP::v4; };
template <> struct IpAddrType<in6_addr> { using type = IP::v6; };
template <class IpAddrStorage> using ip_addr_type_t = typename IpAddrType<IpAddrStorage>::type;

enum class IpAddrParsingError {
    UnknownIpAddrType,
    InvalidFormat,
    IpAddrEmpty,
};
constexpr auto GetErrorMessage(IpAddrParsingError err) -> std::string_view {
    using enum IpAddrParsingError;
    switch(err) {
        case UnknownIpAddrType:
            return "Unknown IP address type";
        case InvalidFormat:
            return "Invalid IP address format";
        case IpAddrEmpty:
            return "Empty IP address";
        default:
            return "Unknown IP address parsing error";
    }
}
template <class OStream>
auto operator<<(OStream&& out, IpAddrParsingError err) -> OStream&& {
    out << GetErrorMessage(err);
    return std::forward<OStream>(out);
}

auto ConstructIpAddrStorage(IpAddr)
  -> std::variant<in_addr, in6_addr, IpAddrParsingError>;


template <class OStream>
inline auto operator<<(OStream&& out, const in_addr& ipv4Addr) -> OStream&& {
    auto buf = std::string(INET_ADDRSTRLEN, 0);
    inet_ntop(AF_INET, &ipv4Addr, buf.data(), buf.size());
    out << buf;
    return std::forward<OStream>(out);
}

template <class OStream>
inline auto operator<<(OStream&& out, const in6_addr& ipv6Addr) -> OStream&& {
    auto buf = std::string(INET6_ADDRSTRLEN, 0);
    inet_ntop(AF_INET6, &ipv6Addr, buf.data(), buf.size());
    out << buf; 
    return std::forward<OStream>(out);
}
