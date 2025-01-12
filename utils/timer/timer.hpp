#pragma once

#include <chrono>


class Timer {
private:
    decltype(std::chrono::steady_clock::now()) StartTime;
    std::chrono::milliseconds Timeout;
public:
    explicit Timer(std::chrono::milliseconds timeout) noexcept; 
    auto CalcRemainingTime() const -> std::chrono::milliseconds;
};
