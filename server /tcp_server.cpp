#include "tcp_server.hpp"

#include "../utils/overloaded.hpp"

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
        if (sockFd == -1) {
            return SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "socket() syscall failed (tcp_server.cpp, line 50)",
            };
        }

        const auto sockAddr = ConstructSockAddr(*ipAddr, serverAddr.Port);
        if (bind(sockFd, (const sockaddr*) &sockAddr, sizeof(sockAddr)) == -1) {
            // TODO: log the error if `close()` returns `-1`
            close(sockFd);
            return SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "bind() syscall failed (tcp_server.cpp, line 53)",
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
    if (ipAddrType != IpAddrType::IPv4 && ipAddrType != IpAddrType::IPv6) {
        return IpAddrParsingError::UnknownIpAddrType;
    }
    auto sockFdOrErr = ipAddrType == IpAddrType::IPv4
                     ? TcpServerCreateNewImpl<IpAddrType::IPv4>(serverAddr)
                     : TcpServerCreateNewImpl<IpAddrType::IPv6>(serverAddr);
    return std::visit(overloaded{
        [](int sockFd) -> R { return TcpServer{sockFd}; },
        [](auto& err ) -> R { return std::move(err); },
    }, sockFdOrErr);
}
