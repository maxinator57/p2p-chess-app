#include "state.hpp"
#include "state_handlers/all.hpp"
#include "tcp_client.hpp"
#include "user_client/console_client.hpp"

#include "../utils/overloaded.hpp"


auto main() -> int {
    const auto tcpClientOrErr = TcpClient::CreateNew(Endpoint{
        .IpAddr = IP::v4::loopback{},
        .Port = 60000
    });
    std::visit(overloaded{ 
        [](const auto& err) {
            std::cerr << "Failed to create TCP client: ";
            LogErrorAndExit(err);
        },
        [](const TcpClient&) {},
    }, tcpClientOrErr);
    const auto& tcpClient = std::get<TcpClient>(tcpClientOrErr);
    
    const auto centralServerEndpoint = Endpoint{
        .IpAddr = IP::v4::loopback{},
        .Port = 60001
    };  
    auto userClient = ConsoleClient{};
    auto state = NState::State{NState::NeedToConnectToCentralServer{}};
    auto updateState = [&state](auto& nextState) mutable {
        state = std::move(nextState);
    };
    for (;;) std::visit(overloaded{
        [&](NState::NeedToConnectToCentralServer) {
            auto nextState = HandleState(
                NState::NeedToConnectToCentralServer{},
                tcpClient,
                centralServerEndpoint
            );
            std::visit(updateState, nextState);
        }, 
        [](std::derived_from<NState::ErrorState> auto& errorState) {
            LogErrorAndExit(errorState.ErrorDescription);
        },
        [&](auto& nonErrorState) {
            auto nextState = HandleState(nonErrorState, tcpClient, userClient);
            std::visit(updateState, nextState);
        },
        [](NState::NeedToExit) {
            exit(EXIT_SUCCESS);
        }
    }, state);
}