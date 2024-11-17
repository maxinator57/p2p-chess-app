#include "../primitives/game_id/game_id.hpp"
#include "../primitives/player_id/player_id.hpp"

#include <variant>


namespace NActions {
    struct CreateNewGame {
        PlayerId Creator;
    };
    enum class CreateNewGameError {
        NoAvailableSpaceInGameDB,
    };
    using CreateNewGameResult = std::variant<GameId, CreateNewGameError>;
} // namespace NActions
