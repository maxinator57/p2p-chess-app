#pragma once


#include "message.hpp"

#include "../primitives/game_id/game_id.hpp"
#include "../primitives/player_id/player_id.hpp"

#include <cstddef>
#include <type_traits>


namespace NApi {
    struct CreateNewGame {
        PlayerId CreatorId;
        struct Result;
        enum class Error : uint8_t {
            NoAvailableSpaceInGameDB = 50,
        };
    };

    struct CreateNewGame::Result
        : public std::variant<GameId, CreateNewGame::Error>
    {
        using std::variant<GameId, CreateNewGame::Error>::variant;
        using Self = CreateNewGame::Result;
        enum class VariantIndex : uint8_t { GameId = 0, Error = 1, };
        static constexpr auto kSerializedSize = sizeof(VariantIndex) + GameId::kSerializedSize;
        auto ToBytes(std::span<std::byte, kSerializedSize>) const noexcept
          -> void;
        struct UnknownVariantIndex {
            std::underlying_type_t<VariantIndex> Value;
        };
        struct UnknownCreateNewGameError {
            std::underlying_type_t<CreateNewGame::Error> Value;
        };
        using ParsingError = std::variant<UnknownVariantIndex, UnknownCreateNewGameError>;
        static auto FromBytes(std::span<std::byte, kSerializedSize>) noexcept
          -> std::variant<Self, ParsingError>;
    };

    template <class OStream>
    inline auto operator<<(OStream& out, CreateNewGame::Error err) -> OStream& {
        switch (err) {
            case CreateNewGame::Error::NoAvailableSpaceInGameDB: {
                out << "CreateNewGame error: no available space in server game database";
                break;
            }
            default: {
                out << "Unknown CreateNewGame error (value: "
                    << static_cast<std::underlying_type_t<decltype(err)>>(err)
                    << ")";
            }
        }
        return out;
    }

    template <class OStream>
    inline auto operator<<(
        OStream& out,
        CreateNewGame::Result::UnknownVariantIndex unknownIdx
    ) -> OStream& {
        out << "CreateNewGame::Result: unknown variant index (value: "
            << unknownIdx.Value << ")";
        return out;
    }

    template <class OStream>
    inline auto operator<<(
        OStream& out,
        CreateNewGame::Result::UnknownCreateNewGameError unknownErr
    ) -> OStream& {
        out << "CreateNewGame::Result: unknown CreateNewGame error (value: "
            << unknownErr.Value << ")";
        return out;
    }

    template <class OStream>
    inline auto operator<<(OStream& out, CreateNewGame::Result::ParsingError err) -> OStream& {
        out << "CreateNewGame::Result::ParsingError: ";
        std::visit([&out](auto err) { out << err.Value; }, err);
        return out;
    }

    template <>
    struct Message<MessageType::CreateNewGameRequest> {
        static constexpr auto kSerializedSize = 0;
    };
    using CreateNewGameRequest = Message<MessageType::CreateNewGameRequest>;

    template <>
    struct Message<MessageType::CreateNewGameResponse> : public CreateNewGame::Result {
        using CreateNewGame::Result::Result; 
    };
    using CreateNewGameResponse = Message<MessageType::CreateNewGameResponse>;
} // namespace NApi
