#include "api.hpp"

#include <utility>


namespace NApi {
    CreateNewGameRequest::CreateNewGameRequest() noexcept
        : Request{.Type = RequestType::CreateNewGame}
    {
    }

    JoinNewGameRequest::JoinNewGameRequest(GameId&& gameId) noexcept
        : Request{.Type = RequestType::JoinNewGame}
        , Id(std::move(gameId))
    {
    }
} // namespace NApi
