#include "all.hpp"

#include "../../api/create_new_game.hpp"
#include "../../utils/overloaded.hpp"
#include "../../utils/to_string_generic.hpp"
#include <chrono>


namespace {
    using namespace NState; 
    using namespace NApi;
    using namespace std::chrono_literals;
    using PossibleNextState =
        std::variant<CreatedNewGame, FailedToCreateNewGame>;
}


auto NState::HandleState(
    NeedToCreateNewGame,
    const TcpClient& tcpClient,
    const IUserClient&
) noexcept -> PossibleNextState {
    const auto msg = Serialize(CreateNewGameRequest{});
    const auto sndResult = tcpClient.Send(msg, 10s);
    return std::visit(overloaded{
        [](const TcpClient::Timeout& timeout) -> PossibleNextState {
            return FailedToCreateNewGame{{
                .ErrorDescription =
                    "the \"create new game\" request to the central server timed out "
                    "(timeout: " + std::to_string(timeout.Duration.count()) + "ms)",
                .RecommendationHowToFix =
                    "try increasing the timeout for sending requests in the app settings",
            }};
        },
        [](TcpClient::ConnectionTerminatedByPeer terminated) -> PossibleNextState {
            return FailedToCreateNewGame{{
                .ErrorDescription = terminated.GetErrorMessage(),
            }};
        },
        [](const SystemError& err) -> PossibleNextState {
            return FailedToCreateNewGame{{
                .ErrorDescription = ToStringGeneric(err),
            }};
        },
        [&tcpClient](TcpClient::Ok) -> PossibleNextState {
            Buf<MessageType::CreateNewGameResponse> rcvBuf;
            const auto rcvResult = tcpClient.Receive(rcvBuf, 10s);
            return std::visit(overloaded{ 
                [](auto& err) -> PossibleNextState {
                    return FailedToCreateNewGame{{
                        .ErrorDescription = ToStringGeneric(err),
                        // TODO: write proper recommendation how to fix
                        .RecommendationHowToFix = std::nullopt,
                    }};
                },
                [&rcvBuf](TcpClient::Ok) -> PossibleNextState {
                    auto respOrErr = Deserialize<MessageType::CreateNewGameResponse>(rcvBuf);
                    return std::visit(overloaded{
                        [](const auto& err) -> PossibleNextState {
                            return FailedToCreateNewGame{{
                                .ErrorDescription = "Got bad response from central server: "
                                                    + ToStringGeneric(err),
                            }};
                        },
                        [](CreateNewGameResponse&& resp) -> PossibleNextState {
                            return std::visit(overloaded{
                                [](CreateNewGame::Error err) -> PossibleNextState {
                                    return FailedToCreateNewGame{{
                                        .ErrorDescription = "The central server could not create the "
                                                            "game due to the following error: "
                                                            + ToStringGeneric(err),
                                    }};
                                },
                                [](GameId&& createdGameId) -> PossibleNextState {
                                    return CreatedNewGame{.Id = std::move(createdGameId).GetValue()};
                                },
                            }, std::move(resp));
                        },
                    }, respOrErr);
                },
            }, rcvResult);
        },
    }, sndResult);
}
