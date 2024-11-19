#pragma once


#include <cstring>
#include <iostream>
#include <system_error>


#define STR(x) STR2(x)
#define STR2(x) #x
#define SOURCE_LOCATION __FILE__ ", line " STR(__LINE__)

template <class Error>
struct ErrorWithContext {
    Error Value;
    std::string_view ContextMessage;
};

using SystemError = ErrorWithContext<std::errc>;
template <class OStream>
inline auto operator<<(OStream& out, const SystemError& err) -> OStream& {
    const auto errorCode = std::make_error_code(err.Value);
    out << err.ContextMessage << " with the following error: "
        << strerrorname_np(errorCode.value())
        << "(" << errorCode.value() << "), description: "
        << errorCode.message();
    return out;
}

template <class Error>
inline auto LogErrorAndExit(Error&& err) {
    std::cerr << err << "\n";
    exit(EXIT_FAILURE);
}
