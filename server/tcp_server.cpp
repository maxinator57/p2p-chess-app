#include "tcp_server.hpp"

#include "../utils/overloaded.hpp"

#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>


TcpServer::TcpServer(int sockFd) noexcept
    : ListeningSockFd_(sockFd)
{
}

TcpServer::TcpServer(TcpServer&& other) noexcept
    : ListeningSockFd_(other.ListeningSockFd_)
{
    // The value `-1` indicates that an instance
    // of `TcpServer` is in a moved-from state
    other.ListeningSockFd_ = -1;
}

TcpServer::~TcpServer() noexcept {
    // Close the underlying file descriptor if
    // this instance of `TcpServer` is not in
    // a moved-from state, i.e. `ListeningSockFd_` != -1
    if (ListeningSockFd_ != -1) {
        // TODO: log the error if `close()` returns `-1`
        close(ListeningSockFd_);
    }
}

template <class IpAddrType>
requires std::same_as<IpAddrType, IP::v4>
      || std::same_as<IpAddrType, IP::v6>
auto TcpServer::CreateNew(const SockAddrData serverAddr) noexcept
  -> std::variant<TcpServer, SystemError, IpAddrParsingError> {
    const auto ipAddrOrErr = ConstructIpAddr<IpAddrType>(serverAddr.IpAddrStr);
    const auto* const ipAddr = std::get_if<ip_addr_storage_t<IpAddrType>>(&ipAddrOrErr);
    if (!ipAddr) return std::move(std::get<IpAddrParsingError>(ipAddrOrErr));
    const auto sockFd = socket(AddressFamily<IpAddrType>(), SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockFd == -1) return SystemError{
        .Value = std::errc{errno},
        .ContextMessage = "socket() syscall failed (" SOURCE_LOCATION ")",
    };
    auto server = TcpServer(sockFd);
    const auto sockAddr = ConstructSockAddr(*ipAddr, serverAddr.Port);
    if (bind(sockFd, (const sockaddr*) &sockAddr, sizeof(sockAddr)) == -1) {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "bind() syscall failed (" SOURCE_LOCATION ")",
        };
    }
    return server;
}
template auto TcpServer::CreateNew<IP::v4>(const SockAddrData serverAddr)
  -> std::variant<TcpServer, SystemError, IpAddrParsingError>;
template auto TcpServer::CreateNew<IP::v6>(const SockAddrData serverAddr)
  -> std::variant<TcpServer, SystemError, IpAddrParsingError>;

auto TcpServer::CreateNew(const SockAddrData serverAddr) noexcept
  -> std::variant<TcpServer, SystemError, IpAddrParsingError> {
    using R = std::variant<TcpServer, SystemError, IpAddrParsingError>;
    auto ipAddrTypeOrErr = DetectIpAddrType(serverAddr.IpAddrStr);
    return std::visit(overloaded{
        [serverAddr](IP::v4) -> R { return TcpServer::CreateNew<IP::v4>(serverAddr); },
        [serverAddr](IP::v6) -> R { return TcpServer::CreateNew<IP::v6>(serverAddr); },
        [](IpAddrParsingError& err) -> R { return std::move(err); },
    }, ipAddrTypeOrErr);
}

auto TcpServer::Listen() const noexcept -> std::optional<SystemError> {
    if (listen(ListeningSockFd_, /* backlog: */ 128) == -1) {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "listen() syscall failed (" SOURCE_LOCATION ")",
        };
    }
    return std::nullopt;
}

auto TcpServer::Accept() const noexcept -> std::variant<ClientId, AcceptWouldBlock, SystemError> {
    auto result = std::variant<ClientId, AcceptWouldBlock, SystemError>{
        std::in_place_type_t<ClientId>{},
    };
    auto& clientId = std::get<ClientId>(result);
    auto sockAddrLen = socklen_t{sizeof(clientId.SockAddr)};
    clientId.ConnSockFd = accept4(
        ListeningSockFd_,
        (sockaddr*) &clientId.SockAddr,
        &sockAddrLen,
        SOCK_NONBLOCK
    );
    if (clientId.ConnSockFd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            result = AcceptWouldBlock{};
        } else {
            result = SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "accept() syscall failed (" SOURCE_LOCATION ")",
            };
        }
    }
    return result;
}

auto TcpServer::GetListeningSockFd() const noexcept {
    return ListeningSockFd_;
}
