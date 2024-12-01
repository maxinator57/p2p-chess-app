#pragma once


#include "state.hpp"
#include "user_actions.hpp"


class ConsoleClient {
public:
    auto ActOn(NState::ConnectedToCentralServer)
      -> std::variant<NUserAction::CreateNewGame, NUserAction::JoinGame>;

    auto ActOn(NState::CreatedNewGame) -> void;
};
