#include "console_client.hpp"
#include "state.hpp"
#include "tcp_client.hpp"
#include "../networking/api.hpp"
#include "../utils/overloaded.hpp"


#include <netinet/in.h>
#include <sys/socket.h>


auto main() -> int {
    const auto tcpClientOrErr = TcpClient::CreateNew({
        .IpAddr = IP::v4::loopback{},
        .Port = 60000
    });
    std::visit(overloaded{ 
        [](const auto& err) {
            std::cerr << "Creating new tcp client: ";
            LogErrorAndExit(err);
        },
        [](const TcpClient&) {},
    }, tcpClientOrErr);
    const auto& tcpClient = std::get<TcpClient>(tcpClientOrErr);

    const auto connOrErr = tcpClient.Connect({
        .IpAddr = IP::v4::loopback{},
        .Port = 60001
    });
    std::visit(overloaded{
        [](const auto& err) {
            std::cerr << "Connect to central server: ";
            LogErrorAndExit(err);
        },
        [](const TcpClient::ConnectionEstablished) {},
    }, connOrErr); 

    auto userClient = ConsoleClient{};
    auto state = State{NState::ConnectedToCentralServer{}};
    for (;;) {
        std::visit(overloaded{
            [&](NState::ConnectedToCentralServer) {
                auto userAction = userClient.ActOn(
                    NState::ConnectedToCentralServer{}
                );
                std::visit(overloaded{
                    [&](NUserAction::CreateNewGame) {
                        const auto resp = tcpClient.Send(
                            NApi::CreateNewGameRequest{}
                        );
                    },
                    [&](NUserAction::JoinGame& game) {
                        const auto resp = tcpClient.Send(
                            NApi::JoinGameRequest{std::move(game.Id)}
                        );
                    },
                }, userAction);
            },
            [&](auto) {}
        }, state);
    }
}