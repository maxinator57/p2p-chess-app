#include "all.hpp"

#include "../tcp_acceptor.hpp"


namespace {
    static constexpr uint8_t kTimeoutInSecondsForPeerToJoinGame = 60;
}


auto NState::HandleState(
    CreatedNewGame& game,
    TcpClient& tcpClient,
    IUserClient& userClient
) noexcept -> std::variant<ConnectedToPeer, FailedToConnectToPeer> {
    userClient.ActOn(game, kTimeoutInSecondsForPeerToJoinGame);
    auto acceptorOrErr = TcpAcceptor::FromTcpClient(std::move(tcpClient));
}