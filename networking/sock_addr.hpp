#pragma once


#include "ip_addr.hpp"

#include <netinet/in.h>


struct Endpoint {
    class MandatoryIpAddr {
    public:
        IpAddr Value;
        MandatoryIpAddr(auto&& value)
            : Value(std::forward<decltype(value)>(value)) {}
        operator IpAddr() const { return Value; }
    } IpAddr;
    // Port in local byte order
    struct MandatoryPort {
        in_port_t Value;
        MandatoryPort(in_port_t value) : Value(value) {}
        operator in_port_t() const { return Value; }
    } Port;
};

auto ConstructSockAddr(
    const in_addr& ipv4Addr,
    in_port_t portInLocalByteOrder
) -> sockaddr_in;

auto ConstructSockAddr(
    const in6_addr& ipv6Addr,
    in_port_t portInLocalByteOrder
) -> sockaddr_in6;


auto operator==(const sockaddr_in& lhs, const sockaddr_in& rhs) -> bool;
auto operator==(const sockaddr_in6& lhs, const sockaddr_in6& rhs) -> bool;
