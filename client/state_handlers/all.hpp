#pragma once


#include "../state.hpp"
#include "../tcp_client.hpp"
#include "../user_client/user_client_interface.hpp"


namespace NState {
    auto HandleState(NeedToConnectToCentralServer, const TcpClient&, Endpoint) noexcept
      -> std::variant<ConnectedToCentralServer, FailedToConnectToCentralServer>;

    auto HandleState(ConnectedToCentralServer, const TcpClient&, IUserClient&) noexcept
      -> std::variant<NeedToCreateNewGame, NeedToJoinGame>;

    auto HandleState(NeedToCreateNewGame, const TcpClient&, const IUserClient&) noexcept
      -> std::variant<CreatedNewGame, FailedToCreateNewGame>; 

    auto HandleState(const NeedToJoinGame&, const TcpClient&, const IUserClient&) noexcept
      -> std::variant<JoinedGame, FailedToJoinGame>;

    auto HandleState(CreatedNewGame&, TcpClient&, IUserClient&) noexcept
      -> std::variant<ConnectedToPeer, FailedToConnectToPeer>;

    auto HandleState(JoinedGame&, const TcpClient&, IUserClient&) noexcept
      -> std::variant<ConnectedToPeer, FailedToConnectToPeer>;

    auto HandleState(const ConnectedToPeer&, const TcpClient&, IUserClient&)
      -> std::variant<NeedToConnectToCentralServer, NeedToExit>;
} // namespace NState
