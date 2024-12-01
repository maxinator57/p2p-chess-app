#pragma once


#include "../primitives/game_id/game_id.hpp"


#include <variant>


namespace NUserAction {
    struct CreateNewGame {};
    struct JoinGame { GameId Id;};
    struct MakeMove {};
    struct OfferDraw {};
    struct Resign {};

    using UserAction = std::variant<
        CreateNewGame,
        JoinGame,
        MakeMove,
        OfferDraw,
        Resign
    >;
} // namespace NActions

using NUserAction::UserAction;
