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

auto TcpServer::GetListeningSockFd() const noexcept {
    return ListeningSockFd_;
}

namespace {
    template <IpAddrType F>
    auto TcpServerCreateNewImpl(const SockAddrData serverAddr)
      -> std::variant<int /* sockFd */, SystemError, IpAddrParsingError> {
        const auto ipAddrOrErr = ConstructIpAddr<F>(serverAddr.IpAddrStr);
        const auto* const ipAddr = std::get_if<ip_addr_storage_t<F>>(&ipAddrOrErr);
        if (!ipAddr) return std::move(std::get<IpAddrParsingError>(ipAddrOrErr));

        const auto sockFd = socket(static_cast<int>(F), SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sockFd == -1) return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "socket() syscall failed (" SOURCE_LOCATION ")",
        };

        const auto sockAddr = ConstructSockAddr(*ipAddr, serverAddr.Port);
        if (bind(sockFd, (const sockaddr*) &sockAddr, sizeof(sockAddr)) == -1) {
            // TODO: log the error if `close()` returns `-1`
            close(sockFd);
            return SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "bind() syscall failed (" SOURCE_LOCATION ")",
            };
        } else {
            return sockFd;
        }
    }
} // anonymous namespace

auto TcpServer::CreateNew(
    const SockAddrData serverAddr,
    std::optional<IpAddrType> optIpAddrType
) noexcept
  -> std::variant<TcpServer, SystemError, IpAddrParsingError> {
    using R = std::variant<TcpServer, SystemError, IpAddrParsingError>;
    auto ipAddrTypeOrErr = optIpAddrType.has_value()
           ? optIpAddrType.value()
           : DetectIpAddrType(serverAddr.IpAddrStr);
    if (auto* err = std::get_if<IpAddrParsingError>(&ipAddrTypeOrErr); err) {
        return std::move(*err);
    }
    const auto ipAddrType = std::get<IpAddrType>(ipAddrTypeOrErr);
    switch (ipAddrType) {
        case IpAddrType::IPv4: [[fallthrough]];
        case IpAddrType::IPv6: {
            auto sockFdOrErr = ipAddrType == IpAddrType::IPv4
                             ? TcpServerCreateNewImpl<IpAddrType::IPv4>(serverAddr)
                             : TcpServerCreateNewImpl<IpAddrType::IPv6>(serverAddr);
            return std::visit(overloaded{
                [](int sockFd) -> R { return TcpServer{sockFd}; },
                [](auto& err ) -> R { return std::move(err); },
            }, sockFdOrErr);
        }
        default:
            return IpAddrParsingError::UnknownIpAddrType;
    }
}

auto TcpServer::Listen() const noexcept -> std::optional<SystemError> {
    if (listen(ListeningSockFd_, 128) == -1) {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "listen() syscall failed (" SOURCE_LOCATION ")",
        };
    } else {
        return std::nullopt;
    }
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
