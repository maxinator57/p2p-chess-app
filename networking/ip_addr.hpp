#pragma once


#include <arpa/inet.h>
#include <netinet/in.h>

#include <string_view>
#include <variant>


enum class IpAddrType {
    IPv4 = AF_INET,
    IPv6 = AF_INET6,
};

template <IpAddrType> struct IpAddrStorage;
template <> struct IpAddrStorage<IpAddrType::IPv4> { using type = in_addr; };
template <> struct IpAddrStorage<IpAddrType::IPv6> { using type = in6_addr; };
template <IpAddrType F> using ip_addr_storage_t = IpAddrStorage<F>::type;

enum class IpAddrParsingError {
    UnknownIpAddrType,
    InvalidFormat,
    IpAddrEmpty,
};
constexpr auto GetErrorMessage(IpAddrParsingError err) -> std::string_view {
    return ""; 
}


template <IpAddrType F>
auto ConstructIpAddr(const char* const ipAddrStr)
  -> std::variant<
         ip_addr_storage_t<F>,
         IpAddrParsingError
     >;
template <>
auto ConstructIpAddr<IpAddrType::IPv4>(const char* ipAddrStr)
  -> std::variant<in_addr, IpAddrParsingError>;
template <>
auto ConstructIpAddr<IpAddrType::IPv6>(const char* ipAddrStr)
  -> std::variant<in6_addr, IpAddrParsingError>;

auto DetectIpAddrType(const char* ipAddrStr)
  -> std::variant<IpAddrType, IpAddrParsingError>;
