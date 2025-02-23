#pragma once


#include "../networking/ip_addr.hpp"
#include "../networking/sock_addr.hpp"
#include "../utils/error.hpp"

#include <chrono>
#include <optional>
#include <span>


namespace NTcpClientActionResult {
    struct Ok {};
    struct ConnectionTerminatedByPeer {
        static auto GetErrorMessage() -> std::string;
    };
    struct Timeout {
        std::chrono::milliseconds Duration;
        std::chrono::milliseconds WallTimeElapsed;
        auto GetErrorMessage(std::string_view prefix) const -> std::string;
    };
} // namespace NTcpClientActionResults

class TcpClient {
    friend class TcpAcceptor;
private:
    int SockFd_;
private:
    explicit TcpClient(int sockFd) noexcept;
public:
    using Ok = NTcpClientActionResult::Ok;
    using ConnectionTerminatedByPeer = NTcpClientActionResult::ConnectionTerminatedByPeer;
    using Timeout = NTcpClientActionResult::Timeout;
public:
    TcpClient(const TcpClient& other) = delete;
    TcpClient(TcpClient&& other) noexcept;
    TcpClient& operator=(TcpClient&& other) noexcept;
    ~TcpClient() noexcept;

    template <class IpAddrType>
    requires std::same_as<IpAddrType, IP::v4>
          || std::same_as<IpAddrType, IP::v6>
    [[nodiscard]] static auto CreateNew() noexcept
      -> std::variant<TcpClient, SystemError>;

    // IP address type is auto-detected
    [[nodiscard]] static auto CreateNew(Endpoint) noexcept
      -> std::variant<TcpClient, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto Connect(Endpoint) const noexcept
      -> std::variant<Ok, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto Disconnect() const noexcept
      -> std::optional<SystemError>;

    [[nodiscard]] auto Send(
        std::span<const std::byte> msg,
        std::chrono::milliseconds timeout
    ) const noexcept
      -> std::variant<Ok, SystemError, Timeout, ConnectionTerminatedByPeer>; 

    [[nodiscard]] auto Receive(
        std::span<std::byte> msg,
        std::chrono::milliseconds timeout
    ) const noexcept
      -> std::variant<Ok, SystemError, Timeout, ConnectionTerminatedByPeer>;
};
