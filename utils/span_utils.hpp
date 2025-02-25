#pragma once


#include <algorithm>
#include <span>
#include <tuple>


template <class T>
struct StdSpanTraits {
    static constexpr auto IsStdSpan = false;
};
template <class T>
struct StdSpanTraits<std::span<T, std::dynamic_extent>> {
    static constexpr auto IsStdSpan = true;
    static constexpr auto HasKnownSize = false;
    using ElementType = T;
};
template <class T>
struct StdSpanTraits<const std::span<T, std::dynamic_extent>> {
    static constexpr auto IsStdSpan = true;
    static constexpr auto HasKnownSize = false;
    using ElementType = T;
};
template <class T, size_t Size_>
struct StdSpanTraits<std::span<T, Size_>> {
    static constexpr auto IsStdSpan = true;
    static constexpr auto HasKnownSize = true;
    static constexpr auto Size = Size_;
    using ElementType = T;
};
template <class T, size_t Size_>
struct StdSpanTraits<const std::span<T, Size_>> {
    static constexpr auto IsStdSpan = true;
    static constexpr auto HasKnownSize = true;
    static constexpr auto Size = Size_;
    using ElementType = T;
};


template <class T> concept IsConvertibleToStdSpan =
    StdSpanTraits<decltype(std::span{std::declval<T&>()})>::IsStdSpan;

template <class T> concept IsConvertibleToStdSpanWithKnownSize =
    StdSpanTraits<decltype(std::span{std::declval<T&>()})>::HasKnownSize;


namespace NSpanSplitImpl {
template <size_t NumArgsToSum, size_t Arg, size_t... Args>
struct Sum {
    static constexpr size_t Value = 
        NumArgsToSum == 0 ? 0 : Arg + Sum<NumArgsToSum - 1, Args...>::Value;
};
template <size_t NumArgsToSum, size_t Arg>
struct Sum<NumArgsToSum, Arg> {
    static constexpr size_t Value = NumArgsToSum == 0 ? 0 : Arg;
};

template <size_t... PartSizes, size_t... Indices>
[[nodiscard]] constexpr auto SplitImpl(std::index_sequence<Indices...>, auto s)
{
    using Traits = StdSpanTraits<decltype(s)>;
    static_assert(Traits::HasKnownSize);
    constexpr auto totalPartsSize = (PartSizes + ...);
    static_assert(totalPartsSize < Traits::Size);
    return std::tuple{
         (s. template subspan<Sum<Indices, PartSizes...>::Value, PartSizes>())...,
         s. template subspan<totalPartsSize, Traits::Size - totalPartsSize>()
    };
}
} // namespace NSpanSplitImpl


template <size_t... PartSizes>
[[nodiscard]] constexpr auto Split(const auto& s)
requires IsConvertibleToStdSpanWithKnownSize<decltype(s)>
{
    return NSpanSplitImpl::SplitImpl<PartSizes...>(
        std::make_index_sequence<sizeof...(PartSizes)>(),
        std::span{s}
    );
}


template <size_t Size>
struct SpanCopyArgs {
    std::span<const std::byte, Size> Src;
    std::span<std::byte, Size> Dst;
};
template <size_t Size>
constexpr auto SpanCopy(SpanCopyArgs<Size> args) -> void {
    std::copy_n(args.Src.begin(), Size, args.Dst.begin());
}


template <class T>
auto MemoryRepresentation(const T& x) -> std::span<const std::byte, sizeof(T)> {
    return std::as_bytes(std::span<const T, 1>(&x, 1));
}
