#include "tcp_server.hpp"

#include "../utils/overloaded.hpp"

#include <cstdlib>
#include <iostream>


auto main() -> int {
    const auto serverOrErr = TcpServer::CreateNew({.Port = 60001}, IpAddrType::IPv4);
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