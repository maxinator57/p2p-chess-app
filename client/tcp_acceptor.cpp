#include "tcp_acceptor.hpp"

#include "tcp_client.hpp"
#include "../utils/timer/timer.hpp"

#include <sys/poll.h>
#include <unistd.h>


TcpAcceptor::TcpAcceptor(int sockFd) noexcept : SockFd_(sockFd) {}

auto TcpAcceptor::FromTcpClient(TcpClient &&tcpClient) noexcept
  -> std::variant<TcpAcceptor, SystemError> {
    if (auto err = tcpClient.Disconnect()) {
        return *err;
    } else if (listen(tcpClient.SockFd_, /* backlog */ 128) == 0) {
        auto acceptor = TcpAcceptor(tcpClient.SockFd_);
        // The value `-1` indicates that an instance
        // of `TcpClient` is in a moved-from state
        tcpClient.SockFd_ = -1;
        return acceptor;
    } else {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "listen() syscall failed (" SOURCE_LOCATION ")",
        };
    }
}

auto TcpAcceptor::AcceptExpectedPeer(
    const sockaddr_storage& expectedPeerAddress,
    const std::chrono::milliseconds timeout
) const noexcept
  -> std::variant<TcpClient, Timeout, SystemError, PeerAddressMismatchError> {
    auto peerAddress = sockaddr_storage{};
    auto peerAddressSize = socklen_t{sizeof(peerAddress)};
    auto pollFd = pollfd{
        .fd = SockFd_,
        .events = POLLIN,
        .revents = 0,
    };
    auto pollResult = -1;
    auto remainingTime = timeout;
    const auto timer = Timer(timeout);
    for (; pollResult != 0; remainingTime = timer.CalcRemainingTime()) {
        pollResult = poll(&pollFd, 1, remainingTime.count());
        if (pollResult == -1) {
            if (errno == EINTR) continue;
            else return SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "poll() syscall failed (" SOURCE_LOCATION ")",
            };
        } else if (pollResult == 1) {
            if (pollFd.revents & POLLIN) {
                const auto peerSockFd = accept4(SockFd_, (sockaddr*) &peerAddress, &peerAddressSize, SOCK_NONBLOCK);
                if (peerSockFd == -1) {
                    if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                        continue;
                    } else {
                        return SystemError{
                            .Value = std::errc{errno},
                            .ContextMessage = "accept4() syscall failed (" SOURCE_LOCATION ")",
                        };
                    }
                } else if (std::memcmp(&peerAddress, &expectedPeerAddress, peerAddressSize) != 0) {
                    close(peerSockFd);
                    return PeerAddressMismatchError{};
                } else {
                    return TcpClient{peerSockFd};
                }
            } else { // POLLERR | POLLHUP | POLLNVAL
                if (pollFd.revents == POLLNVAL) {
                    return SystemError{
                        .Value = std::errc{EBADF},
                        .ContextMessage = "poll() syscall failed (" SOURCE_LOCATION ")",
                    };
                } else {
                    // Impossible
                    // TODO: at least add logging here
                }
            }
        }
    }
    return Timeout{};
}
