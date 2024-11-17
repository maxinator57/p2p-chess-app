#pragma once


#include "../networking/ip_addr.hpp"
#include "../networking/sock_addr.hpp"
#include "../utils/error.hpp"

#include <optional>


class TcpClient {
private:
    int SockFd_;
private:
    explicit TcpClient(int sockFd) noexcept;
public:
    TcpClient(const TcpClient& other) = delete;
    TcpClient(TcpClient&& other) noexcept;
    ~TcpClient() noexcept;

    [[nodiscard]] static auto CreateNew(IpAddrType) noexcept
      -> std::variant<TcpClient, SystemError>;
    [[nodiscard]] static auto CreateNew(SockAddrData, std::optional<IpAddrType> = {}) noexcept
      -> std::variant<TcpClient, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto GetSockFd() const noexcept -> int;

    struct ConnectionEstablished {};
    [[nodiscard]] auto Connect(SockAddrData) const noexcept
      -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;
};