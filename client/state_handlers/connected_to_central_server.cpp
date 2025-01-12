#include "all.hpp"

#include "../../utils/overloaded.hpp"


namespace {
    using namespace NState;
    using PossibleNextState =
        std::variant<NeedToCreateNewGame, NeedToJoinGame>;
}


auto NState::HandleState(
    ConnectedToCentralServer,
    const TcpClient& tcpClient,
    IUserClient& userClient
) noexcept -> PossibleNextState {
    auto userAction = userClient.ActOn(ConnectedToCentralServer{});
    return std::visit(overloaded{
        [&tcpClient](NUserAction::CreateNewGame) -> PossibleNextState {
            return NeedToCreateNewGame{};
        },
        [&tcpClient](NUserAction::JoinGame& game) -> PossibleNextState {
            return NeedToJoinGame{.Id = std::move(game.Id)};
        },
    }, userAction);
}
