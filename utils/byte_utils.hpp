#include <cstddef>


namespace NByteUtils {
constexpr auto operator ""_b(unsigned long long x) -> std::byte {
    return std::byte(x);
}
constexpr auto operator ""_b(char x) -> std::byte {
    return std::byte(x);
}
} // namespace NByteUtils
