#include "../create_new_game.hpp"
#include "../join_game.hpp"
#include "../socket_address.hpp"
#include "../../utils/to_string_generic.hpp"

#include <cassert>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>


struct Ok {};
struct Failed{
    std::string ErrorMessage;
};
struct TestResult : public std::variant<Ok, Failed> {
    using std::variant<Ok, Failed>::variant;
};


using namespace NApi;
template <MessageType MT>
[[nodiscard]] auto RunSerializationTest(const Message<MT>& msg) -> TestResult {
    const auto buf = Serialize(msg);
    const auto deserializedMsgOrErr = Deserialize<MT>(buf);
    const auto* deserializedMsgPtr = std::get_if<Message<MT>>(&deserializedMsgOrErr);
    if (deserializedMsgPtr == nullptr) {
        return Failed{
            (std::stringstream{} << "Error: failed to deserialize message: " << msg).str()
        };
    } else if (msg == *deserializedMsgPtr) {
        return Ok{};
    } else {
        return Failed{
            (std::stringstream{}
                << "Error: deserialized message is different from the original message:\n"
                << "Original message:     " << msg << "\n"
                << "Deserialized message: " << *deserializedMsgPtr
            ).str()
        };
    }
}


template <MessageType MT>
auto RunTestAndPrintResult(const char* testName, const Message<MT>& msg) -> void {                                                      
    auto testResult = RunSerializationTest(msg);
    std::visit(overloaded{                             
        [=](Ok) {                                       
            std::cerr << "Test \"" << testName << "\" OK\n";              
        },                                             
        [=](Failed& failed) {                                
            std::cerr << "Test \"" << testName << "\" FAILED: " 
                      << failed.ErrorMessage << "\n";  
        },                                             
    }, testResult);
}


auto main() -> int {
    RunTestAndPrintResult("CreateNewGameRequest serialization", CreateNewGameRequest{});
    RunTestAndPrintResult("JoinGameRequest serialization", JoinGameRequest{
        .GameIdToJoin = 12345,
    });
    RunTestAndPrintResult("IpV4 SocketAddressMsg serialization", SocketAddressMsg{sockaddr_in{
        .sin_family = AF_INET,
        .sin_port = htons(12345),
        .sin_addr = in_addr{.s_addr = htonl(INADDR_LOOPBACK)},
    }});
    RunTestAndPrintResult("IpV6 SocketAddressMsg serialization", SocketAddressMsg{sockaddr_in6{
        .sin6_family = AF_INET6,
        .sin6_port = htons(54321),
        .sin6_addr = IN6ADDR_LOOPBACK_INIT,
    }});
}
