#include "create_new_game.hpp"

#include "../utils/integer_serialization.hpp"
#include "../utils/overloaded.hpp"
#include "../utils/span_split.hpp"

#include <algorithm>
#include <type_traits>


namespace NApi {
    auto CreateNewGameResponse::ToBytes(
        std::span<std::byte, kSerializedSize> to
    ) const noexcept -> void {
        const auto [lSpan, rSpan] = Split<sizeof(VariantIndex)>(to);
        std::visit(overloaded{
            [lSpan, rSpan](const GameId& gameId) {
                EnumToBytes(VariantIndex::GameId, lSpan);
                gameId.ToBytes(rSpan);
            },
            [lSpan, rSpan](CreateNewGame::Error err) {
                EnumToBytes(VariantIndex::Error, lSpan);
                const auto [mSpan, rest] = Split<sizeof(err)>(rSpan);
                EnumToBytes(err, mSpan);
                std::ranges::fill(rest, std::byte{0});
            }
        }, *this);
    }

    auto CreateNewGameResponse::FromBytes(std::span<const std::byte, kSerializedSize> from) noexcept
      -> std::variant<CreateNewGameResponse, DeserializationError>
    {
        const auto [lSpan, rSpan] = Split<sizeof(VariantIndex)>(from);
        const auto variantIndex = EnumFromBytes<VariantIndex>(lSpan);
        switch (variantIndex) {
            case VariantIndex::GameId: {
                return GameId::FromBytes(rSpan);
            }
            case VariantIndex::Error: {
                const auto [mSpan, rest] = Split<sizeof(CreateNewGame::Error)>(rSpan);
                const auto err = EnumFromBytes<CreateNewGame::Error>(mSpan);
                switch (err) {
                    case CreateNewGame::Error::NoAvailableSpaceInGameDB:
                        return err;
                    default:
                        return UnknownCreateNewGameError{
                            .Value = static_cast<std::underlying_type_t<CreateNewGame::Error>>(err),
                        };
                }
            }
            default: {
                return UnknownVariantIndex{
                    .Value = static_cast<std::underlying_type_t<VariantIndex>>(variantIndex),
                };
            }
        }
    }
} // namespace NApi
