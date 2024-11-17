#include "tcp_client.hpp"

#include <cstdlib>
#include <iostream>
#include <variant>


auto main() -> int {
    const auto clientOrErr = TcpClient::CreateNew({.Port = 60000}, IpAddrType::IPv4);
    if (const auto* err = std::get_if<SystemError>(&clientOrErr); err) {
        std::cout << "Error: " << *err << "\n";
        exit(EXIT_FAILURE);
    }

    const auto& client = std::get<TcpClient>(clientOrErr);
    const auto result = client.Connect({.IpAddrStr = "127.0.0.1", .Port = 60001});
    if (const auto* err = std::get_if<SystemError>(&result); err) {
        std::cout << "Error: " << *err << "\n";
        exit(EXIT_FAILURE);
    } else if (const auto* err = std::get_if<IpAddrParsingError>(&result); err) {
        std::cout << "IpAddrParsingError\n";
        exit(EXIT_FAILURE);
    }
}