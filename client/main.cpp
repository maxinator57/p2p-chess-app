#include "tcp_client.hpp"

#include "../utils/overloaded.hpp"


auto main() -> int {
    const auto clientOrErr = TcpClient::CreateNew<IP::v6>();
    std::visit(overloaded{ 
        [](const auto& err) { LogErrorAndExit(err); },
        [](const TcpClient&) {},
    }, clientOrErr);
    const auto& client = std::get<TcpClient>(clientOrErr);

    const auto result = client.Connect<IP::v6>({.Port = 60001});
    std::visit(overloaded{
        [](const auto& err) { LogErrorAndExit(err); },
        [](TcpClient::ConnectionEstablished) {},
    }, result);
}