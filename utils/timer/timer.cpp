#include "timer.hpp"


namespace {
    using namespace std::chrono;
}

Timer::Timer(const milliseconds timeout) noexcept
    : StartTime(Clock::now())
    , Timeout(timeout)
{
}

auto Timer::CalcElapsedTime() const -> milliseconds {
    const auto curTime = Clock::now();
    return duration_cast<milliseconds>(curTime - StartTime);
}

auto Timer::CalcRemainingTime() const -> milliseconds {
    const auto timeElapsed = CalcElapsedTime();
    return Timeout < timeElapsed ? milliseconds{0} : Timeout - timeElapsed;
}
