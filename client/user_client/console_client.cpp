#include "console_client.hpp"

#include "../../utils/error.hpp"

#include <iostream>
#include <string_view>


auto ConsoleClient::ActOn(NState::ConnectedToCentralServer) noexcept
  -> std::variant<NUserAction::CreateNewGame, NUserAction::JoinGame> {
    static constexpr auto prompt = std::string_view{
        "Connection to the central server established successfully.\n"
        "Type \"new game\" to create a new game or "
        "\"join game <game_id>\" to join an existing game.\n"
        ">>> "
    };
    static constexpr auto newGameCmd = std::string_view{"new game"};
    static constexpr auto joinGameCmd = std::string_view{"join game "};
    std::cout << prompt;
    auto userAction = std::string{};
    for (;;) {
        std::cin >> userAction;
        if (userAction == newGameCmd) {
            return NUserAction::CreateNewGame{};
        } else if (userAction.starts_with(joinGameCmd)) {
            const auto gameIdStr = std::string_view{userAction}.substr(joinGameCmd.size());
            const auto gameIdOrErr = GameId::FromString(gameIdStr);
            // TODO: change impl to `std::visit`
            if (auto* gameIdPtr = std::get_if<GameId>(&gameIdOrErr); gameIdPtr) {
                return NUserAction::JoinGame{.Id = *gameIdPtr};
            } else {
                std::cout << "Couldn't parse game_id from the string \"" << gameIdStr << "\", got the following error: "
                          << std::get<SystemError>(gameIdOrErr);
            }
        } else {
            std::cout << "Couldn't interpret action \"" << userAction << "\".\n";
        }
        std::cout << prompt;
    }
}

auto ConsoleClient::ActOn(
    const NState::CreatedNewGame& game,
    const uint8_t timeoutInSecondsForPeerToJoin
) noexcept -> void {
    std::cout << "Successfully created new game with id "
              << game.Id
              << ".\n"
                 "Tell this game id to the other player and ask them "
                 "to make a \"join game\" request with this game id.\n"
                 "Waiting for the other player to join the game for "
              << timeoutInSecondsForPeerToJoin
              << " seconds...";
}
