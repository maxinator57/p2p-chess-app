#include "tcp_client.hpp"

#include "../networking/sock_addr.hpp"
#include "../utils/overloaded.hpp"

#include <sys/socket.h>
#include <unistd.h>


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

TcpClient::~TcpClient() noexcept {
    // Close the underlying file descriptor if
    // this instance of `TcpClient` is not in
    // a moved-from state, i.e. `SockFd_` != -1
    if (SockFd_ != -1) {
        // TODO: log the error if `close()` returns `-1`
        close(SockFd_);
    }
}

auto TcpClient::GetSockFd() const noexcept -> int {
    return SockFd_;
}

template <class IpAddrType>
requires std::same_as<IpAddrType, IP::v4>
      || std::same_as<IpAddrType, IP::v6>
auto TcpClient::CreateNew() noexcept -> std::variant<TcpClient, SystemError> {
    const auto sockFd = socket(AddressFamily<IpAddrType>(), SOCK_STREAM, 0);
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
                    if (connect(client.GetSockFd(), (sockaddr*) &serverSockAddr, sizeof(serverSockAddr)) == -1) {
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
  -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError> {
    using R = std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;
    auto ipAddrStorageOrErr = ConstructIpAddrStorage(serverEndpoint.IpAddr);
    return std::visit(overloaded{
        [](IpAddrParsingError& err) -> R { return std::move(err); },
        [port = serverEndpoint.Port, this](auto& addr) -> R {
            const auto serverSockAddr = ConstructSockAddr(addr, port);
            if (connect(GetSockFd(), (sockaddr*) &serverSockAddr, sizeof(serverSockAddr)) == -1) {
                return SystemError{
                    .Value = std::errc{errno},
                    .ContextMessage = "connect() syscall failed (" SOURCE_LOCATION ")",
                };
            }
            return TcpClient::ConnectionEstablished{};
        },
    }, ipAddrStorageOrErr);
}

auto TcpClient::Send(NApi::CreateNewGameRequest) const noexcept
  -> std::variant<NApi::CreateNewGameResponse, SystemError> {
}

auto TcpClient::Send(NApi::JoinGameRequest) const noexcept
  -> std::variant<NApi::JoinGameResponse, SystemError> {
}

