#include "api.hpp"

#include <utility>


namespace NApi {
    CreateNewGameRequest::CreateNewGameRequest() noexcept
        : Request(RequestType::CreateNewGame)
    {
    }

    JoinNewGameRequest::JoinNewGameRequest(GameId&& gameId) noexcept
        : Request(RequestType::JoinNewGame)
        , Id(std::move(gameId))
    {
    }
} // namespace NApi
