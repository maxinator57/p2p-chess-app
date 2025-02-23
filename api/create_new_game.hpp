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
            NoAvailableSpaceInGameDB = 1,
        };
    };

    struct CreateNewGame::Result
        : public std::variant<GameId, CreateNewGame::Error>
    {
        using std::variant<GameId, CreateNewGame::Error>::variant; 
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

    template <>
    struct Message<MessageType::CreateNewGameRequest> {
        static constexpr auto kSerializedSize = 0;
    };
    using CreateNewGameRequest = Message<MessageType::CreateNewGameRequest>;

    template <>
    struct Message<MessageType::CreateNewGameResponse> : public CreateNewGame::Result {
        using CreateNewGame::Result::Result;
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
        using DeserializationError = std::variant<
            UnknownVariantIndex,
            UnknownCreateNewGameError
        >;
        static auto FromBytes(std::span<const std::byte, kSerializedSize>) noexcept
          -> std::variant<Message, DeserializationError>;
    };
    using CreateNewGameResponse = Message<MessageType::CreateNewGameResponse>;

    template <class OStream>
    inline auto operator<<(
        OStream& out,
        CreateNewGameResponse::UnknownVariantIndex unknownIdx
    ) -> OStream& {
        out << "CreateNewGameResponse: unknown variant index (value: "
            << unknownIdx.Value << ")";
        return out;
    }

    template <class OStream>
    inline auto operator<<(
        OStream& out,
        CreateNewGameResponse::UnknownCreateNewGameError unknownErr
    ) -> OStream& {
        out << "CreateNewGameResponse: unknown CreateNewGame error (value: "
            << unknownErr.Value << ")";
        return out;
    }

    template <class OStream>
    inline auto operator<<(OStream& out, CreateNewGameResponse::DeserializationError err) -> OStream& {
        out << "CreateNewGameResponse::DeserializationError: ";
        std::visit([&out](auto err) { out << err.Value; }, err);
        return out;
    }
} // namespace NApi
