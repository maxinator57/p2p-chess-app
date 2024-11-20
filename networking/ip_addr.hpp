#pragma once


#include <arpa/inet.h>
#include <netinet/in.h>

#include <string_view>
#include <sys/socket.h>
#include <variant>


struct IP {
    struct v4 {};
    struct v6 {};
};

template <class IpAddrType> constexpr auto AddressFamily();
template <> constexpr auto AddressFamily<IP::v4>() { return AF_INET; }
template <> constexpr auto AddressFamily<IP::v6>() { return AF_INET6; }


template <class IpAddrType> struct IpAddrStorage;
template <> struct IpAddrStorage<IP::v4> { using type = in_addr; };
template <> struct IpAddrStorage<IP::v6> { using type = in6_addr; };
template <class IpAddrType> using ip_addr_storage_t = typename IpAddrStorage<IpAddrType>::type;

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
auto operator<<(OStream& out, IpAddrParsingError err) -> OStream& {
    out << GetErrorMessage(err);
    return out;
}

template <class IpAddrType>
auto ConstructIpAddr(const char* const ipAddrStr)
  -> std::variant<ip_addr_storage_t<IpAddrType>, IpAddrParsingError>;
template <>
auto ConstructIpAddr<IP::v4>(const char* ipAddrStr)
  -> std::variant<in_addr, IpAddrParsingError>;
template <>
auto ConstructIpAddr<IP::v6>(const char* ipAddrStr)
  -> std::variant<in6_addr, IpAddrParsingError>;

auto DetectIpAddrType(const char* ipAddrStr)
  -> std::variant<IP::v4, IP::v6, IpAddrParsingError>;
