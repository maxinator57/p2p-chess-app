#pragma once


#include <sstream>


template <class T>
concept IsWritableToStringstream = requires (T x) {
    { std::stringstream{} << x } -> std::same_as<std::stringstream&&>;
};

template <class T>
inline auto ToStringGeneric(const T& x) -> std::string {
    static_assert(IsWritableToStringstream<T>);
    return (std::stringstream{} << x).str();
}
