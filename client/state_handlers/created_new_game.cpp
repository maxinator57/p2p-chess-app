#include "all.hpp"

#include "../../api/socket_address.hpp"
#include "../../utils/integer_serialization.hpp"
#include "../../utils/overloaded.hpp"
#include "../../utils/to_string_generic.hpp"

#include <chrono>


namespace {
    // TODO: read this timeout value from config
    using namespace std::chrono_literals;
    static constexpr auto kTimeoutForPeerToJoinGame = 60s;
    static constexpr auto kTimeoutForPeerToConnect = 60s;
    using PossibleNextState = std::variant<
        NState::NeedToAcceptConnectionFromExpectedPeer,
        NState::FailedToEstablishConnectionWithPeer
    >;
}


auto NState::HandleState(
    CreatedNewGame& game,
    const TcpClient& tcpClient,
    IUserClient& userClient
) noexcept -> PossibleNextState {
    userClient.ActOn(game, kTimeoutForPeerToJoinGame);
    using namespace NApi;
    Buf<MessageType::SocketAddress> rcvBuf;
    const auto rcvResult = tcpClient.Receive(rcvBuf, kTimeoutForPeerToJoinGame);
    return std::visit(overloaded{ 
        [](const SystemError& err) -> PossibleNextState {
            return FailedToEstablishConnectionWithPeer{{
                .ErrorDescription = ToStringGeneric(err),
            }};
        },
        [](const TcpClient::Timeout& timeout) -> PossibleNextState {
            return FailedToEstablishConnectionWithPeer{{
                .ErrorDescription = timeout.GetErrorMessage(
                    "waiting for the peer to join the game and for the central server to send the peer address"
                ),
            }};
        },
        [](TcpClient::ConnectionTerminatedByPeer terminated) -> PossibleNextState {
            return FailedToEstablishConnectionWithPeer{{
                .ErrorDescription = terminated.GetErrorMessage(),
            }};
        },
        [&rcvBuf](TcpClient::Ok) -> PossibleNextState {
            auto peerAddressOrErr = Deserialize<MessageType::SocketAddress>(rcvBuf);
            return std::visit(overloaded{
                []()
            }, peerAddressOrErr);
            return NeedToAcceptConnectionFromExpectedPeer{
                .PeerAddress = expectedPeerAddress.addr,
            };
        },
    }, rcvResult);
}
