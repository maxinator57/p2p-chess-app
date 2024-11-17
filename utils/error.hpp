#pragma once


#include <cstring>
#include <system_error>


template <class Error>
struct ErrorWithContext {
    Error Value;
    std::string_view ContextMessage;
};

using SystemError = ErrorWithContext<std::errc>;
template <class OStream>
auto operator<<(OStream& out, const SystemError& err) -> OStream& {
    const auto errorCode = std::make_error_code(err.Value);
    out << err.ContextMessage << " with the following error: "
        << strerrorname_np(errorCode.value())
        << "(" << errorCode.value() << "), description: "
        << errorCode.message();
    return out;
}
