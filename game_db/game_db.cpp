#include "game_db.hpp"


auto GameDB::ProcessAction(NActions::CreateNewGame a) -> NActions::CreateNewGameResult {
    auto gameData = GameData{
        .FstPlayerId = std::move(a.Creator)
    };
    if (Storage_.size() == kMaxStorageSize_) {
        return NActions::CreateNewGameError::NoAvailableSpaceInGameDB;
    }
    // The expected number of iterations of this loop is constant
    for (;;) {
        auto newGameId = GameId::CreateRandom();
        // TODO: add logging here
        const auto [it, inserted] = Storage_.try_emplace(
            std::move(newGameId),
            std::move(gameData) 
        );
        if (inserted) {
            return newGameId;
        }
    }
}

auto GameDB::ProcessAction(NActions::AddPlayerToGame a) -> NActions::AddPlayerToGameResult {
    using enum NActions::AddPlayerToGameResult;
    const auto it = Storage_.find(a.Game);
    if (it == Storage_.end()) {
        return GameIdDoesNotExist;
    } else if (auto& gameData = it->second; gameData.SndPlayerId.has_value()) {
        return GameAlreadyHasTwoPlayers;   
    } else {
        gameData.SndPlayerId = std::move(a.Player);
        return Success;
    }
}
