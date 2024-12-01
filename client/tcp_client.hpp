#pragma once


#include "../networking/api.hpp"
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

    [[nodiscard]] auto GetSockFd() const noexcept -> int;

    template <class IpAddrType>
    requires std::same_as<IpAddrType, IP::v4>
          || std::same_as<IpAddrType, IP::v6>
    [[nodiscard]] static auto CreateNew() noexcept
      -> std::variant<TcpClient, SystemError>;

    // IP address type is auto-detected
    [[nodiscard]] static auto CreateNew(Endpoint) noexcept
      -> std::variant<TcpClient, SystemError, IpAddrParsingError>;

    struct ConnectionEstablished {};
    // IP address type is auto-detected
    [[nodiscard]] auto Connect(Endpoint) const noexcept
      -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto Send(NApi::CreateNewGameRequest) const noexcept
      -> std::variant<NApi::CreateNewGameResponse, SystemError>;
    [[nodiscard]] auto Send(NApi::JoinGameRequest) const noexcept
      -> std::variant<NApi::JoinGameResponse, SystemError>;
};