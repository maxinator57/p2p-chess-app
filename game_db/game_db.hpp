#pragma once


#include "../actions/create_new_game.hpp"
#include "../actions/add_player_to_game.hpp"

#include <map>
#include <optional>


class GameDB {
private:
    struct GameData {
        PlayerId FstPlayerId;
        std::optional<PlayerId> SndPlayerId = std::nullopt;
    };
    std::map<GameId, GameData> Storage_;
    static constexpr auto kMaxStorageSize_ = size_t{100};
public:
    auto ProcessAction(NActions::CreateNewGame) -> NActions::CreateNewGameResult;
    auto ProcessAction(NActions::AddPlayerToGame) -> NActions::AddPlayerToGameResult;
};