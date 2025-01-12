#include "all.hpp"

#include "../../api/create_new_game.hpp"
#include "../../utils/overloaded.hpp"
#include "../../utils/to_string_generic.hpp"


namespace {
    using namespace NState;
    using PossibleNextState =
        std::variant<CreatedNewGame, FailedToCreateNewGame>;
}


auto NState::HandleState(
    NeedToCreateNewGame,
    const TcpClient& tcpClient,
    const IUserClient&
) noexcept -> PossibleNextState {
    if (auto err = tcpClient.Send(NApi::CreateNewGameRequest{})) {
        return FailedToCreateNewGame{{
            .ErrorDescription = ToStringGeneric(*err),
            .RecommendationHowToFix =
                "The error occurred when sending a \"create new game\" "
                "request to the central server. "
                "Maybe the central server is down or something "
                "is wrong with tcp client settings.",
        }};
    }
    auto respOrErr = tcpClient.Receive<NApi::MessageType::CreateNewGameResponse>();
    return std::visit(overloaded{
        [](NApi::CreateNewGameResponse& resp) -> PossibleNextState {
            return std::visit(overloaded{
                [](GameId& newGameId) -> PossibleNextState {
                    return CreatedNewGame{.Id = std::move(newGameId)};
                },
                [](NApi::CreateNewGame::Error err) -> PossibleNextState {
                    return FailedToCreateNewGame{{
                        .ErrorDescription = ToStringGeneric(err),
                        // TODO: write proper recommendation how to fix
                        .RecommendationHowToFix = std::nullopt,
                    }};
                },
            }, resp);
        },
        [](auto& err) -> PossibleNextState {
            return FailedToCreateNewGame{{
                .ErrorDescription = ToStringGeneric(err),
                // TODO: write proper recommendation how to fix
                .RecommendationHowToFix = std::nullopt,
            }};
        },
    }, respOrErr);
}
