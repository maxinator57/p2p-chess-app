#pragma once


#include "../networking/ip_addr.hpp"
#include "../networking/sock_addr.hpp"
#include "../utils/error.hpp"

#include <optional>
#include <sys/socket.h>
#include <variant>


class TcpServer {
private:
    int ListeningSockFd_;
private:
    explicit TcpServer(int sockFd) noexcept;
public:
    TcpServer(const TcpServer& other) = delete;
    TcpServer(TcpServer&& other) noexcept;
    ~TcpServer() noexcept;

    template <class IpAddrType>
    requires std::same_as<IpAddrType, IP::v4>
          || std::same_as<IpAddrType, IP::v6>
    [[nodiscard]] static auto CreateNew(SockAddrData) noexcept
      -> std::variant<TcpServer, SystemError, IpAddrParsingError>;

    /* IP address type is auto-detected */
    [[nodiscard]] static auto CreateNew(SockAddrData) noexcept
      -> std::variant<TcpServer, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto Listen() const noexcept
      -> std::optional<SystemError>;

    struct ClientId {
        sockaddr_storage SockAddr;
        int ConnSockFd;
    };
    struct AcceptWouldBlock {};
    [[nodiscard]] auto Accept() const noexcept
      -> std::variant<ClientId, AcceptWouldBlock, SystemError>;

    [[nodiscard]] auto GetListeningSockFd() const noexcept;
};