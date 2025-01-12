#pragma once


#include "user_client_interface.hpp"


class ConsoleClient : public IUserClient {
public:
    auto ActOn(NState::ConnectedToCentralServer) noexcept
      -> std::variant<NUserAction::CreateNewGame, NUserAction::JoinGame> override;
    auto ActOn(
        const NState::CreatedNewGame&,
        uint8_t timeoutInSecondsForPeerToJoin
    ) noexcept -> void override;
    auto ActOn(const NState::FailedToJoinGame&) noexcept -> void override;
};
