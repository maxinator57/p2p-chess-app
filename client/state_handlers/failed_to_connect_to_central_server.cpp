#include "all.hpp"


auto NState::HandleState(
    FailedToConnectToCentralServer&& failed,
    IUserClient& userClient
) noexcept -> void {
    userClient.ActOn(std::move(failed));
}
