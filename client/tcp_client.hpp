#pragma once


#include "../networking/ip_addr.hpp"
#include "../networking/sock_addr.hpp"
#include "../utils/error.hpp"


class TcpClient {
private:
    int SockFd_;
private:
    explicit TcpClient(int sockFd) noexcept;
public:
    TcpClient(const TcpClient& other) = delete;
    TcpClient(TcpClient&& other) noexcept;
    ~TcpClient() noexcept;

    template <class IpAddrType>
    requires std::same_as<IpAddrType, IP::v4>
          || std::same_as<IpAddrType, IP::v6>
    [[nodiscard]] static auto CreateNew() noexcept
      -> std::variant<TcpClient, SystemError>;

    template <class IpAddrType>
    requires std::same_as<IpAddrType, IP::v4>
          || std::same_as<IpAddrType, IP::v6>
    [[nodiscard]] static auto CreateNew(SockAddrData) noexcept
      -> std::variant<TcpClient, SystemError, IpAddrParsingError>;

    // IP address type is auto-detected
    [[nodiscard]] static auto CreateNew(SockAddrData) noexcept
      -> std::variant<TcpClient, SystemError, IpAddrParsingError>;

    struct ConnectionEstablished {};
    template <class IpAddrType>
    requires std::same_as<IpAddrType, IP::v4>
          || std::same_as<IpAddrType, IP::v6>
    [[nodiscard]] auto Connect(SockAddrData) const noexcept
      -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;

    // IP address type is auto-detected
    [[nodiscard]] auto Connect(SockAddrData) const noexcept
      -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto GetSockFd() const noexcept -> int;
};