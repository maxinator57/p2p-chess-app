#pragma once


#include "../state.hpp"
#include "../tcp_client.hpp"
#include "../user_client/user_client_interface.hpp"


namespace NState {
    auto HandleState(NeedToConnectToCentralServer, const TcpClient&, Endpoint) noexcept
      -> std::variant<ConnectedToCentralServer, FailedToConnectToCentralServer>;

    auto HandleState(FailedToConnectToCentralServer&&, IUserClient&) noexcept
      -> void;

    auto HandleState(ConnectedToCentralServer, const TcpClient&, IUserClient&) noexcept
      -> std::variant<NeedToCreateNewGame, NeedToJoinGame>;

    auto HandleState(NeedToCreateNewGame, const TcpClient&, const IUserClient&) noexcept
      -> std::variant<CreatedNewGame, FailedToCreateNewGame>; 

    auto HandleState(const NeedToJoinGame&, const TcpClient&, const IUserClient&) noexcept
      -> std::variant<JoinedGame, FailedToJoinGame>;

    auto HandleState(CreatedNewGame&, const TcpClient&, IUserClient&) noexcept
      -> std::variant<NeedToAcceptConnectionFromExpectedPeer, FailedToEstablishConnectionWithPeer>;

    auto HandleState(const NeedToAcceptConnectionFromExpectedPeer&, TcpClient&, IUserClient&) noexcept
      -> std::variant<EstablishedConnectionWithPeer, NeedToConnectToPeer, FailedToEstablishConnectionWithPeer>;

    auto HandleState(JoinedGame&, const TcpClient&, IUserClient&) noexcept
      -> std::variant<NeedToConnectToPeer, FailedToEstablishConnectionWithPeer>;

    auto HandleState(const NeedToConnectToPeer&, TcpClient&, IUserClient&) noexcept
      -> std::variant<EstablishedConnectionWithPeer, NeedToAcceptConnectionFromExpectedPeer, FailedToEstablishConnectionWithPeer>;

    auto HandleState(const EstablishedConnectionWithPeer&, const TcpClient&, IUserClient&) noexcept
      -> std::variant<NeedToConnectToCentralServer, NeedToExit>;
} // namespace NState
