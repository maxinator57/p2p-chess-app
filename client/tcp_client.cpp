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

template <class IpAddrType>
requires std::same_as<IpAddrType, IP::v4>
      || std::same_as<IpAddrType, IP::v6>
auto TcpClient::CreateNew(const SockAddrData clientAddr) noexcept
  -> std::variant<TcpClient, SystemError, IpAddrParsingError> {
    const auto addrOrErr = ConstructIpAddr<IpAddrType>(clientAddr.IpAddrStr);
    const auto* const addr = std::get_if<ip_addr_storage_t<IpAddrType>>(&addrOrErr);
    if (!addr) return IpAddrParsingError::InvalidFormat;

    auto clientOrErr = TcpClient::CreateNew<IpAddrType>();
    auto* client = std::get_if<TcpClient>(&clientOrErr);
    if (!client) return std::move(std::get<SystemError>(clientOrErr));

    const auto sockAddr = ConstructSockAddr(*addr, clientAddr.Port);
    if (bind(client->GetSockFd(), (const sockaddr*) &sockAddr, sizeof(sockAddr)) == -1) {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "bind() syscall failed (" SOURCE_LOCATION ")",
        };
    }
    return std::move(*client);
}
template auto TcpClient::CreateNew<IP::v4>(SockAddrData) noexcept
  -> std::variant<TcpClient, SystemError, IpAddrParsingError>;
template auto TcpClient::CreateNew<IP::v6>(SockAddrData) noexcept
  -> std::variant<TcpClient, SystemError, IpAddrParsingError>;

auto TcpClient::CreateNew(const SockAddrData clientAddr) noexcept
  -> std::variant<TcpClient, SystemError, IpAddrParsingError> {
    using R = std::variant<TcpClient, SystemError, IpAddrParsingError>;
    auto ipAddrTypeOrErr = DetectIpAddrType(clientAddr.IpAddrStr);
    return std::visit(overloaded{
        [clientAddr](IP::v4) -> R { return TcpClient::CreateNew<IP::v4>(clientAddr); },
        [clientAddr](IP::v6) -> R { return TcpClient::CreateNew<IP::v6>(clientAddr); },
        [](IpAddrParsingError& err) -> R { return std::move(err); },
    }, ipAddrTypeOrErr);
}

template <class IpAddrType>
requires std::same_as<IpAddrType, IP::v4>
      || std::same_as<IpAddrType, IP::v6>
auto TcpClient::Connect(const SockAddrData serverAddr) const noexcept
  -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError> {
    const auto ipAddrOrErr = ConstructIpAddr<IpAddrType>(serverAddr.IpAddrStr);
    if (const auto* err = std::get_if<IpAddrParsingError>(&ipAddrOrErr); err) {
        return *err;
    }
    const auto serverSockAddr = ConstructSockAddr(
        std::get<ip_addr_storage_t<IpAddrType>>(ipAddrOrErr),
        serverAddr.Port
    ); 
    if (connect(SockFd_, (sockaddr*) &serverSockAddr, sizeof(serverSockAddr)) == -1) {
        return SystemError{
            .Value = std::errc{errno},
            .ContextMessage = "connect() syscall failed (" SOURCE_LOCATION ")",
        };
    }
    return TcpClient::ConnectionEstablished{};
}
template auto TcpClient::Connect<IP::v4>(const SockAddrData serverAddr) const noexcept
  -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;
template auto TcpClient::Connect<IP::v6>(const SockAddrData serverAddr) const noexcept
  -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;

auto TcpClient::Connect(const SockAddrData serverAddr) const noexcept
  -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError> {
    using R = std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;
    auto ipAddrTypeOrErr = DetectIpAddrType(serverAddr.IpAddrStr);
    return std::visit(overloaded{
        [serverAddr, this](IP::v4) -> R { return Connect<IP::v4>(serverAddr); },
        [serverAddr, this](IP::v6) -> R { return Connect<IP::v6>(serverAddr); },
        [](IpAddrParsingError& err) -> R { return std::move(err); },
    }, ipAddrTypeOrErr); 
}

auto TcpClient::GetSockFd() const noexcept -> int {
    return SockFd_;
}
