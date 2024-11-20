#include "tcp_server.hpp"

#include "../utils/overloaded.hpp"


auto main() -> int {
    const auto serverOrErr = TcpServer::CreateNew<IP::v6>({.Port = 60001});
    std::visit(overloaded{
        [](const auto& err) { LogErrorAndExit(err); },
        [](const TcpServer&) {}
    }, serverOrErr);
    const auto& server = std::get<TcpServer>(serverOrErr);

    const auto err = server.Listen();
    if (err) LogErrorAndExit(*err);

    for (bool stop = false; !stop;) {
        const auto clientIdOrErr = server.Accept();
        std::visit(overloaded{
            [](TcpServer::AcceptWouldBlock) {},
            [](const auto& err) { LogErrorAndExit(err); },
            [&stop](const TcpServer::ClientId& clientId) {
                std::cout << "Accepted connection from client\n";
                stop = true;
            }
        }, clientIdOrErr);
    }
}