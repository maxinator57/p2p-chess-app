#pragma once


#include "../primitives/game_id/game_id.hpp"
#include "tcp_client.hpp"

#include <variant>


namespace NState {
    struct ErrorState {
        std::string ErrorDescription;
        std::optional<std::string> RecommendationHowToFix = std::nullopt;
    };

    struct NeedToConnectToCentralServer {};
    struct ConnectedToCentralServer {};
    struct FailedToConnectToCentralServer : public ErrorState {
        FailedToConnectToCentralServer(ErrorState s) : ErrorState(std::move(s)) {}
    };

    struct NeedToCreateNewGame {};
    struct CreatedNewGame { GameId Id; };
    struct FailedToCreateNewGame : public ErrorState {
        FailedToCreateNewGame(ErrorState s) : ErrorState(std::move(s)) {}
    };

    struct NeedToJoinGame { GameId Id; };
    struct JoinedGame { GameId Id; };
    struct FailedToJoinGame : public ErrorState {
        FailedToJoinGame(ErrorState s) : ErrorState(std::move(s)) {}
    };

    struct ConnectedToPeer {
        TcpClient Client;
    };
    struct FailedToConnectToPeer : public ErrorState {
        FailedToConnectToPeer(ErrorState s) : ErrorState(std::move(s)) {}
    };

    struct NeedToExit {};

    using State = std::variant<
        NeedToConnectToCentralServer,
        ConnectedToCentralServer,
        FailedToConnectToCentralServer,
        NeedToCreateNewGame,
        CreatedNewGame,
        FailedToCreateNewGame,
        NeedToJoinGame,
        JoinedGame,
        FailedToJoinGame,
        ConnectedToPeer,
        FailedToConnectToPeer,
        NeedToExit
    >;
} // namespace NState
