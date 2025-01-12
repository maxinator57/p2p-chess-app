#include "tcp_acceptor.hpp"
#include "tcp_client.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>


TcpAcceptor::TcpAcceptor(int sockFd) noexcept : SockFd_(sockFd) {}

auto TcpAcceptor::FromTcpClient(TcpClient &&tcpClient) noexcept
  -> std::variant<TcpAcceptor, SystemError> {
    if (auto err = tcpClient.Disconnect(); !err) { 
        auto acceptor = TcpAcceptor(tcpClient.SockFd_);
        // The value `-1` indicates that an instance
        // of `TcpClient` is in a moved-from state
        tcpClient.SockFd_ = -1;
        return acceptor;
    } else {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "connect() syscall failed (" SOURCE_LOCATION ")",
        };
    }
}

auto TcpAcceptor::Listen() const noexcept -> std::optional<SystemError> {
    if (listen(SockFd_, 128) == 0) {
        return std::nullopt;
    } else {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "listen() syscall failed (" SOURCE_LOCATION ")",
        };
    }
}

auto TcpAcceptor::AcceptExpectedPeer(
    uint8_t timeoutInSeconds,
    const sockaddr_storage& expectedPeerAddress
) const noexcept
  -> std::variant<TcpClient, SystemError, AcceptTimedOutError, AcceptPeerAddressMismatchError> {
    auto pollFd = pollfd{
        .fd = SockFd_,
        .events = POLLIN,
        .revents = 0,
    };
    constexpr auto kNumMillisecondsInSecond = 1000;
    if (poll(&pollFd, 1, timeoutInSeconds * kNumMillisecondsInSecond) == 0) {
        return AcceptTimedOutError{};
    }
    auto peerAddress = sockaddr_storage{};
    auto addressSize = socklen_t{sizeof(peerAddress)};
    if (auto peerFd = accept(SockFd_, (sockaddr*) &peerAddress, &addressSize); peerFd != -1) {
        if (std::memcmp(&peerAddress, &expectedPeerAddress, sizeof(expectedPeerAddress)) != 0) {
            // The address of the connected peer does not match the expected peer address
            close(peerFd);
            return AcceptPeerAddressMismatchError{};
        } else {
            return TcpClient{peerFd};
        }
    } else {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "accept() syscall failed (" SOURCE_LOCATION ")",
        };
    }
}