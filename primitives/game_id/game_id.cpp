#include "game_id.hpp"

#include <random>


auto GameId::CreateRandom() noexcept -> GameId {
    static constexpr auto kRngSeed = 57;
    static auto rng = std::mt19937_64{kRngSeed};
    return GameId(rng());
}
