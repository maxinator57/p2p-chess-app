#pragma once

#include <chrono>


class Timer {
public:
    using Clock = std::chrono::steady_clock;
private:
    decltype(Clock::now()) StartTime;
    std::chrono::milliseconds Timeout;
public:
    explicit Timer(std::chrono::milliseconds timeout) noexcept;
    auto CalcElapsedTime() const -> std::chrono::milliseconds;
    auto CalcRemainingTime() const -> std::chrono::milliseconds;
};
