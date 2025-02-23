#pragma once


#include "user_client_interface.hpp"


class ConsoleClient : public IUserClient {
public:
    auto ActOn(NState::FailedToConnectToCentralServer&&) noexcept
      -> void override;
    auto ActOn(NState::ConnectedToCentralServer) noexcept
      -> std::variant<NUserAction::CreateNewGame, NUserAction::JoinGame> override;
    auto ActOn(const NState::CreatedNewGame&, std::chrono::seconds timeoutForPeerToJoin) noexcept
      -> void override;
    auto ActOn(const NState::FailedToJoinGame&) noexcept
      -> void override;
};
