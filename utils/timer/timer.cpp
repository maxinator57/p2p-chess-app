#include "timer.hpp"


Timer::Timer(std::chrono::milliseconds timeout) noexcept
    : StartTime(std::chrono::steady_clock::now())
    , Timeout(timeout)
{
}

auto Timer::CalcRemainingTime() const -> std::chrono::milliseconds {
    const auto curTime = std::chrono::steady_clock::now();
    const auto timeElapsed =
    std::chrono::duration_cast<std::chrono::milliseconds>(curTime - StartTime);
    return Timeout < timeElapsed ? std::chrono::milliseconds{0} : Timeout - timeElapsed;
}
