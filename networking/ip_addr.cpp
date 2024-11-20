#include "ip_addr.hpp"


template <>
auto ConstructIpAddr<IP::v4>(const char* const ipAddrStr)
  -> std::variant<in_addr, IpAddrParsingError> {
    if (ipAddrStr == nullptr) {
        return in_addr{.s_addr = INADDR_ANY};
    } else {
        in_addr addr;
        return inet_aton(ipAddrStr, &addr) != 0
            ? std::variant<in_addr, IpAddrParsingError>{addr}
            : IpAddrParsingError::InvalidFormat;
    }
}

template <>
auto ConstructIpAddr<IP::v6>(const char* const ipAddrStr)
  -> std::variant<in6_addr, IpAddrParsingError> {
    if (ipAddrStr == nullptr) {
        return in6addr_any;
    } else {
        in6_addr addr;
        return inet_pton(AF_INET6, ipAddrStr, &addr) != 0
            ? std::variant<in6_addr, IpAddrParsingError>{addr}
            : IpAddrParsingError::InvalidFormat;
    }
}

auto DetectIpAddrType(const char* ipAddrStr)
  -> std::variant<IP::v4, IP::v6, IpAddrParsingError> {
    if (ipAddrStr == nullptr) {
        return IpAddrParsingError::IpAddrEmpty;
    }
    const auto ipv4AddrOrErr = ConstructIpAddr<IP::v4>(ipAddrStr);
    if (std::holds_alternative<in_addr>(ipv4AddrOrErr)) {
        return IP::v4{};
    }
    const auto ipv6AddrOrErr = ConstructIpAddr<IP::v6>(ipAddrStr);
    if (std::holds_alternative<in6_addr>(ipv6AddrOrErr)) {
        return IP::v6{};
    }
    return IpAddrParsingError::InvalidFormat;
}
