#include "tcp_client.hpp"

#include "../networking/sock_addr.hpp"
#include "../utils/overloaded.hpp"
#include "../utils/robust_read_write/robust_read_write.hpp"

#include <concepts>
#include <sys/socket.h>
#include <unistd.h>


namespace NTcpClientActionResult {
    auto ConnectionTerminatedByPeer::GetErrorMessage() -> std::string {
        return "TCP connection was terminated by peer";
    }
    auto Timeout::GetErrorMessage(std::string_view prefix) const -> std::string {
        return std::string{prefix}
            + " timed out (timeout duration: " + std::to_string(Duration.count()) + "ms, "
            + "actual time elapsed: " + std::to_string(WallTimeElapsed.count()) + "ms)";
    }
}


TcpClient::TcpClient(int sockFd) noexcept
    : SockFd_(sockFd)
{
}

TcpClient::TcpClient(TcpClient&& other) noexcept
    : SockFd_(other.SockFd_)
{
    // The value `-1` indicates that an instance
    // of `TcpClient` is in a moved-from state
    other.SockFd_ = -1;
}

TcpClient& TcpClient::operator=(TcpClient&& other) noexcept {
    std::swap(SockFd_, other.SockFd_);
    return *this;
}

TcpClient::~TcpClient() noexcept {
    // Close the underlying file descriptor if
    // this instance of `TcpClient` is not in
    // a moved-from state, i.e. `SockFd_` != -1
    if (SockFd_ != -1) {
        // TODO: log the error if `close()` returns `-1`
        close(SockFd_);
    }
}

template <class IpAddrType>
requires std::same_as<IpAddrType, IP::v4>
      || std::same_as<IpAddrType, IP::v6>
auto TcpClient::CreateNew() noexcept -> std::variant<TcpClient, SystemError> {
    const auto sockFd = socket(AddressFamily<IpAddrType>(), SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockFd == -1) {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "socket() syscall failed (" SOURCE_LOCATION ")",
        };
    }
    return TcpClient{sockFd};
}
template auto TcpClient::CreateNew<IP::v4>() noexcept -> std::variant<TcpClient, SystemError>;
template auto TcpClient::CreateNew<IP::v6>() noexcept -> std::variant<TcpClient, SystemError>;

auto TcpClient::CreateNew(const Endpoint clientEndpoint) noexcept
  -> std::variant<TcpClient, SystemError, IpAddrParsingError> {
    using R = std::variant<TcpClient, SystemError, IpAddrParsingError>;
    auto ipAddrStorageOrErr = ConstructIpAddrStorage(clientEndpoint.IpAddr);
    return std::visit(overloaded{
        [](IpAddrParsingError& err) -> R { return std::move(err); },
        [port = clientEndpoint.Port](auto& addr) -> R {
            auto clientOrErr = TcpClient::CreateNew<
                ip_addr_type_t<std::remove_reference_t<decltype(addr)>>
            >();
            return std::visit(overloaded{
                [&addr, port](TcpClient& client) -> R {
                    const auto serverSockAddr = ConstructSockAddr(addr, port);
                    if (connect(client.SockFd_, (sockaddr*) &serverSockAddr, sizeof(serverSockAddr)) == -1) {
                        return SystemError{
                            .Value = std::errc{errno},
                            .ContextMessage = "connect() syscall failed (" SOURCE_LOCATION ")",
                        };
                    }
                    return std::move(client);
                },
                [](auto& err) -> R { return std::move(err); },
            }, clientOrErr);
        },
    }, ipAddrStorageOrErr); 
}

auto TcpClient::Connect(const Endpoint serverEndpoint) const noexcept
  -> std::variant<Ok, SystemError, IpAddrParsingError> {
    using R = std::variant<Ok, SystemError, IpAddrParsingError>;
    auto ipAddrStorageOrErr = ConstructIpAddrStorage(serverEndpoint.IpAddr);
    return std::visit(overloaded{
        [](IpAddrParsingError& err) -> R { return std::move(err); },
        [port = serverEndpoint.Port, this](auto& addr) -> R {
            const auto serverSockAddr = ConstructSockAddr(addr, port);
            if (connect(SockFd_, (sockaddr*) &serverSockAddr, sizeof(serverSockAddr)) == -1) {
                return SystemError{
                    .Value = std::errc{errno},
                    .ContextMessage = "connect() syscall failed (" SOURCE_LOCATION ")",
                };
            }
            return Ok{};
        },
    }, ipAddrStorageOrErr);
}

auto TcpClient::Disconnect() const noexcept -> std::optional<SystemError> {
    auto dummyAddr = sockaddr{.sa_family = AF_UNSPEC};
    if (connect(SockFd_, &dummyAddr, sizeof(dummyAddr)) == 0) {
        return std::nullopt; 
    } else {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "connect() syscall failed (" SOURCE_LOCATION ")",
        };
    }
}

auto TcpClient::Send(
    std::span<const std::byte> msg,
    std::chrono::milliseconds timeout
) const noexcept
  -> std::variant<Ok, SystemError, Timeout, ConnectionTerminatedByPeer> {
    using R = std::variant<Ok, SystemError, Timeout, ConnectionTerminatedByPeer>;
    const auto result = RobustSyncWrite(SockFd_, msg, timeout);
    // TODO: add proper logging
    using namespace NRobustSyncWrite;
    return std::visit(overloaded{
        [](OnSuccess) -> R {
            return Ok{};
        },
        [](const OnSystemError& onErr) -> R {
            return onErr.Err;
        },
        [](const OnTimeout& onTimeout) -> R {
            return Timeout{
                .WallTimeElapsed = onTimeout.Timeout,
            };
        },
        [](const OnPollerrOrPollhup& onPollerrOrPollhup) -> R {
            return ConnectionTerminatedByPeer{};
        },
    }, result);
}

auto TcpClient::Receive(
    const std::span<std::byte> msg,
    const std::chrono::milliseconds timeout
) const noexcept
  -> std::variant<Ok, SystemError, Timeout, ConnectionTerminatedByPeer> {
    using R = std::variant<Ok, SystemError, Timeout, ConnectionTerminatedByPeer>;
    const auto result = RobustSyncRead(SockFd_, msg, timeout);
    // TODO: add proper logging
    using namespace NRobustSyncRead;
    return std::visit(overloaded{
        [](OnSuccess) -> R {
            return Ok{};
        },
        [](const OnSystemError& onErr) -> R {
            return onErr.Err;
        },
        [](const OnTimeout& onTimeout) -> R {
            return Timeout{
                .WallTimeElapsed = onTimeout.WallTimeElapsed,
            };
        },
        [](const auto& otherResult) -> R {
            return ConnectionTerminatedByPeer{};
        },
    }, result);
}
