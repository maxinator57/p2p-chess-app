#include "sock_addr.hpp"

#include <netinet/in.h>


auto ConstructSockAddr(
    const in_addr& ipv4Addr,
    in_port_t portInLocalByteOrder
) -> sockaddr_in {
    return {
        .sin_family = AF_INET,
        .sin_port = htons(portInLocalByteOrder),
        .sin_addr = ipv4Addr,
    };
}

auto ConstructSockAddr(
    const in6_addr& ipv6Addr,
    in_port_t portInLocalByteOrder
) -> sockaddr_in6 {
    return {
        .sin6_family = AF_INET6,
        .sin6_port = htons(portInLocalByteOrder),
        .sin6_addr = ipv6Addr,
    };
}
