#include "tcp_client.hpp"

#include "../networking/sock_addr.hpp"

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

auto TcpClient::CreateNew(const IpAddrType F) noexcept
  -> std::variant<TcpClient, SystemError> {
    const auto sockFd = socket(static_cast<int>(F), SOCK_STREAM, 0);
    if (sockFd == -1) {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "socket() syscall failed (" SOURCE_LOCATION ")",
        };
    } else {
        return TcpClient{sockFd};
    }
}

auto TcpClient::GetSockFd() const noexcept -> int {
    return SockFd_;
}

namespace {
    template <IpAddrType F>
    auto TcpClientCreateNewImpl(const SockAddrData clientAddr) noexcept
      -> std::variant<TcpClient, SystemError, IpAddrParsingError> {
        const auto addrOrErr = ConstructIpAddr<F>(clientAddr.IpAddrStr);
        const auto* const addr = std::get_if<ip_addr_storage_t<F>>(&addrOrErr);
        if (!addr) return IpAddrParsingError::InvalidFormat;

        const auto sockAddr = ConstructSockAddr(*addr, clientAddr.Port);

        auto clientOrErr = TcpClient::CreateNew(F);
        auto* client = std::get_if<TcpClient>(&clientOrErr);
        if (!client) return std::move(std::get<SystemError>(clientOrErr)); 

        if (bind(client->GetSockFd(), (const sockaddr*) &sockAddr, sizeof(sockAddr)) == -1) {
            return SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "bind() syscall failed (" SOURCE_LOCATION ")",
            };
        } else {
            return std::move(*client);
        }
    }

    template <IpAddrType F>
    auto TcpClientConnectImpl(const int sockFd, const SockAddrData serverAddr) noexcept
      -> std::variant<TcpClient::ConnectionEstablished, SystemError, IpAddrParsingError> {
        const auto ipAddrOrErr = ConstructIpAddr<F>(serverAddr.IpAddrStr);
        if (const auto* err = std::get_if<IpAddrParsingError>(&ipAddrOrErr); err) {
            return *err;
        }
        const auto serverSockAddr = ConstructSockAddr(
            std::get<ip_addr_storage_t<F>>(ipAddrOrErr),
            serverAddr.Port
        ); 
        if (connect(sockFd, (sockaddr*) &serverSockAddr, sizeof(serverSockAddr)) == -1) {
            return SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "connect() syscall failed (" SOURCE_LOCATION ")",
            };
        } else {
            return TcpClient::ConnectionEstablished{};
        }
    }
} // anonymous namespace

auto TcpClient::CreateNew(
    const SockAddrData clientAddr,
    const std::optional<IpAddrType> optIpAddrType
) noexcept
  -> std::variant<TcpClient, SystemError, IpAddrParsingError> {
    auto F = optIpAddrType.has_value()
           ? optIpAddrType.value()
           : DetectIpAddrType(clientAddr.IpAddrStr);
    if (auto* err = std::get_if<IpAddrParsingError>(&F); err) {
        return std::move(*err);
    }
    switch (std::get<IpAddrType>(F)) {
        case IpAddrType::IPv4:
            return TcpClientCreateNewImpl<IpAddrType::IPv4>(clientAddr);
        case IpAddrType::IPv6:
            return TcpClientCreateNewImpl<IpAddrType::IPv6>(clientAddr);
        default:
            return IpAddrParsingError::UnknownIpAddrType;
    }
}

auto TcpClient::Connect(const SockAddrData serverAddr) const noexcept
  -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError> {
    auto F = DetectIpAddrType(serverAddr.IpAddrStr);
    if (auto* err = std::get_if<IpAddrParsingError>(&F); err) {
        return std::move(*err);
    }
    switch (std::get<IpAddrType>(F)) {
        case IpAddrType::IPv4:
            return TcpClientConnectImpl<IpAddrType::IPv4>(SockFd_, serverAddr);
        case IpAddrType::IPv6:
            return TcpClientConnectImpl<IpAddrType::IPv6>(SockFd_, serverAddr);
        default:
            return IpAddrParsingError::UnknownIpAddrType;
    }
}
