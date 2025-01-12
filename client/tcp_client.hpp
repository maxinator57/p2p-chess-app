#pragma once


#include "../api/message.hpp"
#include "../networking/ip_addr.hpp"
#include "../networking/sock_addr.hpp"
#include "../utils/error.hpp"
#include "../utils/integer_serialization.hpp"
#include "../utils/overloaded.hpp"
#include "../utils/robust_read_write/robust_read_write.hpp"
#include "../utils/to_string_generic.hpp"

#include <optional>
#include <sys/poll.h>


class TcpClient {
    friend class TcpAcceptor;
private:
    int SockFd_;
private:
    explicit TcpClient(int sockFd) noexcept;
public:
    TcpClient(const TcpClient& other) = delete;
    TcpClient(TcpClient&& other) noexcept;
    TcpClient& operator=(TcpClient&& other) noexcept;
    ~TcpClient() noexcept;

    template <class IpAddrType>
    requires std::same_as<IpAddrType, IP::v4>
          || std::same_as<IpAddrType, IP::v6>
    [[nodiscard]] static auto CreateNew() noexcept
      -> std::variant<TcpClient, SystemError>;

    // IP address type is auto-detected
    [[nodiscard]] static auto CreateNew(Endpoint) noexcept
      -> std::variant<TcpClient, SystemError, IpAddrParsingError>;

    struct ConnectionEstablished {};
    // IP address type is auto-detected
    [[nodiscard]] auto Connect(Endpoint) const noexcept
      -> std::variant<ConnectionEstablished, SystemError, IpAddrParsingError>;

    [[nodiscard]] auto Disconnect() const noexcept
      -> std::optional<SystemError>;

    template <NApi::MessageType MessageType>
    [[nodiscard]] auto Send(
        const NApi::Message<MessageType>& msg,
        const std::chrono::milliseconds timeout
    ) const noexcept
      -> std::optional<GenericError> {
        using R = std::optional<GenericError>;
        using Msg = NApi::Message<MessageType>;
        std::array<std::byte, sizeof(MessageType) + Msg::kSerializedSize> buf;
        const auto [fstSpan, sndSpan] = Split<sizeof(MessageType)>(buf);
        EnumToBytes(MessageType, fstSpan);
        if constexpr (Msg::kSerializedSize != 0) msg.ToBytes(sndSpan);
        const auto result = RobustSyncWrite(SockFd_, buf, timeout);
        // TODO: add proper logging
        using namespace NRobustSyncWrite;
        return std::visit(overloaded{
            [](OnSuccess) -> R {
                return std::nullopt;
            },
            [](const OnSystemError& onErr) -> R {
                return GenericError{
                    .Value = ToStringGeneric(onErr.Err),
                };
            },
            [](const OnTimeout& onTimeout) -> R {
                return GenericError{
                    .Value = "sending message central server timed out (the timeout was "
                             + std::to_string(onTimeout.Timeout.count()) + " milliseconds)"
                };
            },
            [](const OnPollerrOrPollhup& onPollerrOrPollhup) -> R {
                return GenericError{
                    .Value = "connection was dropped by the central server"
                };
            },
        }, result);
    }

    template <NApi::MessageType ExpectedMsgType>
    using ReceiveReturnType = std::variant<NApi::Message<ExpectedMsgType>, GenericError>;
    template <NApi::MessageType ExpectedMsgType>
    [[nodiscard]] auto Receive(const std::chrono::milliseconds timeout) const noexcept
      -> ReceiveReturnType<ExpectedMsgType> {
        using R = ReceiveReturnType<ExpectedMsgType>;
        using Msg = NApi::Message<ExpectedMsgType>;
        std::array<std::byte, sizeof(ExpectedMsgType) + Msg::kSerializedSize> buf;
        const auto result = RobustSyncRead(SockFd_, buf);
        // TODO: add proper logging
        using namespace NRobustSyncRead;
        std::visit(overloaded{
            [](OnSuccess) {},
        }, result);
        const auto [lSpan, rSpan] = Split<sizeof(ExpectedMsgType)>(buf);
        const auto msgType = EnumFromBytes<NApi::MessageType>(lSpan);
        // TODO: finish this function
    }
};
