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
    using namespace NApi;
    const auto sndBuf = NApi::Serialize(NApi::JoinGameRequest{game.Id});
    const auto sndResult = tcpClient.Send(sndBuf, std::chrono::seconds{10});
    if (!std::get_if<TcpClient::Ok>(&sndResult)) {
        return FailedToJoinGame{{
            .ErrorDescription = std::visit(overloaded{
                [](const SystemError& err) {
                    return ToStringGeneric(err);
                },
                [](const TcpClient::Timeout& timeout) {
                    return timeout.GetErrorMessage("sending a \"join game request\" message to central server");
                },
                [](const TcpClient::ConnectionTerminatedByPeer& terminated) {
                    return terminated.GetErrorMessage();
                },
                // Redundant, but is needed for the code to compile
                [](TcpClient::Ok) { return std::string{}; },
            }, sndResult),
        }};
    }
    Buf<MessageType::JoinGameResponse> rcvBuf;
    const auto rcvResult = tcpClient.Receive(rcvBuf, std::chrono::seconds{10});
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
    }, rcvResult);
}
