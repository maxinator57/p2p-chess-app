#include "sock_addr.hpp"

#include <cstring>
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

auto operator==(const sockaddr_in& lhs, const sockaddr_in& rhs) -> bool {
    return lhs.sin_port == rhs.sin_port
        && lhs.sin_addr.s_addr == rhs.sin_addr.s_addr;
}

auto operator==(const sockaddr_in6& lhs, const sockaddr_in6& rhs) -> bool {
    return lhs.sin6_port == rhs.sin6_port
        && std::memcmp(
            lhs.sin6_addr.s6_addr,
            rhs.sin6_addr.s6_addr,
            sizeof(in6_addr)
        ) == 0;
}
