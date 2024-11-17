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

    [[nodiscard]] static auto CreateNew(SockAddrData, std::optional<IpAddrType> = {}) noexcept
      -> std::variant<TcpServer, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto GetListeningSockFd() const noexcept;

    [[nodiscard]] auto Listen() const noexcept
      -> std::optional<SystemError>;

    struct ClientId {
        sockaddr_storage IpAddr;
        int ConnSockFd;
    };
    [[nodiscard]] auto Accept() const noexcept
      -> std::variant<ClientId, SystemError>;
};