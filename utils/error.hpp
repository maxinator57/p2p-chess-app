#pragma once


#include <cstring>
#include <iostream>
#include <optional>
#include <system_error>


#define STR(x) STR2(x)
#define STR2(x) #x
#define SOURCE_LOCATION __FILE__ ", line " STR(__LINE__)


template <class Error>
struct ErrorWithContext {
    Error Value;
    std::optional<std::string> ContextMessage = std::nullopt;
};

using SystemError = ErrorWithContext<std::errc>;
template <class OStream>
inline auto operator<<(OStream& out, const SystemError& err) -> OStream& {
    if (err.ContextMessage) {
        out << (*err.ContextMessage) << ", got the following error: ";
    }
    const auto errorCode = std::make_error_code(err.Value);
    out << strerrorname_np(errorCode.value())
        << "(" << errorCode.value() << "), description: "
        << errorCode.message();
    return out;
}


using GenericError = ErrorWithContext<std::string>;
template <class OStream>
inline auto operator<<(OStream& out, const GenericError& err) -> OStream& {
    if (err.ContextMessage) {
        out << err.ContextMessage << ", got the following error: ";
    }
    return out << err.Value;
}

template <class Error>
inline auto LogErrorAndExit(Error&& err) {
    std::cerr << err << "\n";
    exit(EXIT_FAILURE);
}
