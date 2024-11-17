#include "ip_addr.hpp"


template <>
auto ConstructIpAddr<IpAddrType::IPv4>(const char* const ipAddrStr)
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
auto ConstructIpAddr<IpAddrType::IPv6>(const char* const ipAddrStr)
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
  -> std::variant<IpAddrType, IpAddrParsingError> {
    if (ipAddrStr == nullptr) {
        return IpAddrParsingError::IpAddrEmpty;
    }
    const auto ipv4AddrOrErr = ConstructIpAddr<IpAddrType::IPv4>(ipAddrStr);
    if (std::holds_alternative<in_addr>(ipv4AddrOrErr)) {
        return IpAddrType::IPv4;
    }
    const auto ipv6AddrOrErr = ConstructIpAddr<IpAddrType::IPv6>(ipAddrStr);
    if (std::holds_alternative<in6_addr>(ipv6AddrOrErr)) {
        return IpAddrType::IPv6;
    }
    return IpAddrParsingError::InvalidFormat;
}
