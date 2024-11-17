#include "../primitives/game_id/game_id.hpp"
#include "../primitives/player_id/player_id.hpp"


namespace NActions {
    struct AddPlayerToGame {
        PlayerId Player;
        GameId Game;
    };
    enum class AddPlayerToGameResult {
        Success,
        GameIdDoesNotExist,
        GameAlreadyHasTwoPlayers,
    };
} // namespace NActions
