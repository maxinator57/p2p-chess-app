#pragma once


#include "../primitives/game_id/game_id.hpp"

#include <variant>


namespace NState {
    struct ConnectedToCentralServer {};
    struct JoinedGame { GameId Id; };
    struct CreatedNewGame { GameId Id; };

    using State = std::variant<
        ConnectedToCentralServer,
        JoinedGame,
        CreatedNewGame
    >;
} // namespace NState

using NState::State;
