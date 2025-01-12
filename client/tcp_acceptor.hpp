#pragma once


#include "tcp_client.hpp"


class TcpAcceptor {
private:
    int SockFd_;
    explicit TcpAcceptor(int sockFd) noexcept;
public:
    static auto FromTcpClient(TcpClient&& tcpClient) noexcept
      -> std::variant<TcpAcceptor, SystemError>;
    auto Listen() const noexcept -> std::optional<SystemError>;
    struct AcceptTimedOutError {};
    struct AcceptPeerAddressMismatchError {};
    auto AcceptExpectedPeer(
      uint8_t timeoutInSeconds,
      const sockaddr_storage& expectedPeerAddress
    ) const noexcept
      -> std::variant<TcpClient, SystemError, AcceptTimedOutError, AcceptPeerAddressMismatchError>;
};