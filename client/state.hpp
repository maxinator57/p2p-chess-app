#pragma once


#include "../primitives/game_id/game_id.hpp"
#include "tcp_client.hpp"

#include <sys/socket.h>
#include <variant>


namespace NState {
    struct ErrorState {
        std::string ErrorDescription;
        std::optional<std::string> RecommendationHowToFix;
    };

    struct NeedToConnectToCentralServer {};
    struct ConnectedToCentralServer {};
    struct FailedToConnectToCentralServer : public ErrorState {
        explicit FailedToConnectToCentralServer(ErrorState s)
            : ErrorState(std::move(s)) {}
    };

    struct NeedToCreateNewGame {};
    struct CreatedNewGame { GameId Id; };
    struct FailedToCreateNewGame : public ErrorState {
        explicit FailedToCreateNewGame(ErrorState s)
            : ErrorState(std::move(s)) {}
    };

    struct NeedToJoinGame { GameId Id; };
    struct JoinedGame { GameId Id; };
    struct FailedToJoinGame : public ErrorState {
        explicit FailedToJoinGame(ErrorState s)
            : ErrorState(std::move(s)) {}
    };

    struct NeedToAcceptConnectionFromExpectedPeer {
        sockaddr_storage PeerAddress;
        bool AlreadyTriedToConnectToPeer = false;
    };
    struct NeedToConnectToPeer {
        sockaddr_storage PeerAddress;
        bool AlreadyTriedToAcceptConnectionFromPeer = false;
    };
    struct EstablishedConnectionWithPeer {
        TcpClient Client;
    };
    struct FailedToEstablishConnectionWithPeer : public ErrorState {
        explicit FailedToEstablishConnectionWithPeer(ErrorState s)
            : ErrorState(std::move(s)) {}
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
        NeedToAcceptConnectionFromExpectedPeer,
        NeedToConnectToPeer,
        EstablishedConnectionWithPeer,
        FailedToEstablishConnectionWithPeer,
        NeedToExit
    >;
} // namespace NState
