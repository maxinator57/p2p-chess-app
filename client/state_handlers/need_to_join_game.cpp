#include "all.hpp"

#include "../../api/join_game.hpp"
#include "../../utils/overloaded.hpp"
#include "../../utils/to_string_generic.hpp"


namespace {
    using namespace NState;
    using PossibleNextState =
        std::variant<JoinedGame, FailedToJoinGame>;
}

auto NState::HandleState(
    const NState::NeedToJoinGame& game,
    const TcpClient& tcpClient,
    const IUserClient&
) noexcept -> PossibleNextState {
    if (auto err = tcpClient.Send(NApi::JoinGameRequest{game.Id})) {
        LogErrorAndExit(*err);
    }
    auto respOrErr = tcpClient.Receive<NApi::MessageType::JoinGameResponse>();
    return std::visit(overloaded{
        [&game](const NApi::JoinGameResponse& resp) -> PossibleNextState {
            using enum NApi::AddPlayerToGameOp::Result;
            switch (resp.Result) {
                // TODO: write proper recommendations how to fix
                case Success:
                    return JoinedGame{.Id = std::move(game.Id)};
                case GameIdDoesNotExist:
                    return FailedToJoinGame{{
                        .ErrorDescription = "Game id [" + game.Id.ToString() + "] does not exist on the server",
                        .RecommendationHowToFix = std::nullopt,
                    }};
                case GameAlreadyHasTwoPlayers:
                    return FailedToJoinGame{{
                        .ErrorDescription = "Game id [" + game.Id.ToString() + "] already has two players",
                        .RecommendationHowToFix = std::nullopt,
                    }};
                default:
                    return FailedToJoinGame{{
                        .ErrorDescription = "Unknown error ("
                                + std::to_string(ToUnderlying(resp.Result))
                                + ") occurred when trying to join game id [" + game.Id.ToString() + "], ",
                        .RecommendationHowToFix = std::nullopt,
                    }};
            }
        },
        [](const auto& err) -> PossibleNextState {
            return FailedToJoinGame{ErrorState{
                .ErrorDescription = ToStringGeneric(err),
                // TODO: write proper recommendation how to fix
                .RecommendationHowToFix = std::nullopt,
            }};
        }
    }, respOrErr);
}
