#pragma once


#include <bit>
#include <climits>
#include <concepts>
#include <cstring>
#include <span>
#include <type_traits>


// Serialization and deserialization
// correctness relies on this.
static_assert(std::endian::native == std::endian::little);
static_assert(CHAR_BIT == 8);


template <std::integral I>
auto IntToBytes(I x, std::span<std::byte, sizeof(I)> to) -> void {
    std::memcpy(to.data(), &x, sizeof(x));
}
template <std::integral I>
[[nodiscard]] auto IntFromBytes(std::span<const std::byte, sizeof(I)> from) -> I {
    I x;
    std::memcpy(&x, from.data(), sizeof(x));
    return x;
}

template <class E>
requires std::is_enum_v<E>
auto ToUnderlying(E x) -> std::underlying_type_t<E> {
    return static_cast<std::underlying_type_t<E>>(x);
}

template <class E>
requires std::is_enum_v<E>
auto EnumToBytes(E x, std::span<std::byte, sizeof(E)> to) -> void {
    IntToBytes(ToUnderlying(x), to);
}
template <class E>
requires std::is_enum_v<E>
[[nodiscard]] auto EnumFromBytes(std::span<const std::byte, sizeof(E)> from) -> E {
    return static_cast<E>(IntFromBytes<std::underlying_type_t<E>>(from));
}
