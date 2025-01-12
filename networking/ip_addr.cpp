#include "ip_addr.hpp"

#include "../utils/overloaded.hpp"
#include <netinet/in.h>
#include <sys/socket.h>


auto ConstructIpAddrStorage(IpAddr ipAddr)
  -> std::variant<in_addr, in6_addr, IpAddrParsingError> {
    using R = std::variant<in_addr, in6_addr, IpAddrParsingError>;
    return std::visit(overloaded{
        [](std::string_view addrStr) -> R {
            if (addrStr.empty()) return IpAddrParsingError::IpAddrEmpty;
            else { 
                if (in_addr addr; inet_aton(addrStr.data(), &addr) == 1) {
                    return addr;
                }
                if (in6_addr addr; inet_pton(AF_INET6, addrStr.data(), &addr) == 1) {
                    return addr;
                }
                return IpAddrParsingError::InvalidFormat;
            }
        },
        [](IP::v4::any) -> R { return in_addr{.s_addr = INADDR_ANY}; },
        [](IP::v6::any) -> R { return in6addr_any; },
        [](IP::v4::loopback) -> R { return in_addr{.s_addr = INADDR_LOOPBACK}; },
        [](IP::v6::loopback) -> R { return in6addr_loopback; },
    }, ipAddr);
}
