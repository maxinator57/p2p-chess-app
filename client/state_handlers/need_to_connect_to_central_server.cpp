#include "all.hpp"

#include "../../utils/overloaded.hpp"
#include "../../utils/to_string_generic.hpp"


namespace {
    using namespace NState;
    using PossibleNextState =
        std::variant<ConnectedToCentralServer, FailedToConnectToCentralServer>;
}


auto NState::HandleState(
    NeedToConnectToCentralServer,
    const TcpClient& tcpClient,
    Endpoint centralServerEndpoint
) noexcept -> PossibleNextState {
    const auto connOrErr = tcpClient.Connect(centralServerEndpoint);
    return std::visit(overloaded{
        [](const auto& err) -> PossibleNextState {
            return FailedToConnectToCentralServer{{
                .ErrorDescription = ToStringGeneric(err),
                // TODO: write recommendation how to fix
                .RecommendationHowToFix = std::nullopt,
            }};
        },
        [](TcpClient::Ok) -> PossibleNextState {
            return ConnectedToCentralServer{};
        },
    }, connOrErr);
}
