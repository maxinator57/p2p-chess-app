#pragma once


#include <netinet/in.h>


struct SockAddrData {
    const char* IpAddrStr = nullptr;
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
