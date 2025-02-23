#pragma once


#include "../state.hpp"
#include "../user_actions.hpp"


class IUserClient {
public:
    virtual ~IUserClient() = default;
    virtual auto ActOn(NState::FailedToConnectToCentralServer&&) noexcept -> void = 0;
    virtual auto ActOn(NState::ConnectedToCentralServer) noexcept
      -> std::variant<NUserAction::CreateNewGame, NUserAction::JoinGame> = 0;
    virtual auto ActOn(const NState::CreatedNewGame&, std::chrono::seconds timeoutForPeerToJoin) noexcept
      -> void = 0;
    virtual auto ActOn(const NState::FailedToJoinGame&) noexcept -> void = 0;
};
