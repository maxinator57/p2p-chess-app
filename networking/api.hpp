#pragma once


#include "../actions/add_player_to_game.hpp"
#include "../actions/create_new_game.hpp"
#include "../primitives/game_id/game_id.hpp"


namespace NApi {
    enum class RequestType {
        CreateNewGame,
        JoinNewGame,
    };
    struct Request {
        RequestType Type; 
    };

    struct CreateNewGameRequest : public Request {
    public:
        CreateNewGameRequest() noexcept;
    };
    struct CreateNewGameResponse {
        NActions::CreateNewGameResult Result;        
    };

    struct JoinNewGameRequest : public Request {
        GameId Id; 
        explicit JoinNewGameRequest(GameId&& gameId) noexcept; 
    };
    struct JoinNewGameResponse {
        NActions::AddPlayerToGameResult Result;
    };
} // namespace NApi