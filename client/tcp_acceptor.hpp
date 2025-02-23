#pragma once


#include "tcp_client.hpp"


class TcpAcceptor {
private:
    int SockFd_;
    explicit TcpAcceptor(int sockFd) noexcept;
public:
    static auto FromTcpClient(TcpClient&& tcpClient) noexcept
      -> std::variant<TcpAcceptor, SystemError>;
    struct Timeout {};
    struct PeerAddressMismatchError {};
    auto AcceptExpectedPeer(
        const sockaddr_storage& expectedPeerAddress,
        const std::chrono::milliseconds timeout
    ) const noexcept
      -> std::variant<TcpClient, Timeout, SystemError, PeerAddressMismatchError>;
};