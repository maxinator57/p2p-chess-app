#include "tcp_client.hpp"

#include "../utils/overloaded.hpp"

#include <cstdlib>
#include <variant>


auto main() -> int {
    const auto clientOrErr = TcpClient::CreateNew({.Port = 60000}, IpAddrType::IPv4);
    std::visit(overloaded{ 
        [](const auto& err) { LogErrorAndExit(err); },
        [](const TcpClient&) {},
    }, clientOrErr);
    const auto& client = std::get<TcpClient>(clientOrErr);

    const auto result = client.Connect({.IpAddrStr = "127.0.0.1", .Port = 60001});
    std::visit(overloaded{
        [](const auto& err) { LogErrorAndExit(err); },
        [](TcpClient::ConnectionEstablished) {},
    }, result);
}