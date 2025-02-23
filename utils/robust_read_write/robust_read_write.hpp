#pragma once


#include "../error.hpp"

#include <chrono>
#include <span>
#include <variant>


namespace NRobustSyncRead {
    // Indicates that the whole buffer `to` was
    // read from `nonBlockingFd` successfully
    struct OnSuccess {
        size_t NumBytesRead;
        std::chrono::milliseconds WallTimeElapsed;
    };
    // Indicates that this blocking read was cancelled by some
    // other event (i.e. a user pressing a "cancel" button)
    struct OnCancellation {
        size_t NumBytesRead;
        std::chrono::milliseconds WallTimeElapsed;
    };
    // Indicates that a `EOF` occurred (i.e. at some point the underlying
    // `read()` syscall returned 0) before the whole buffer `to` was read
    struct OnPrematureEof {
        size_t NumBytesRead;
        std::chrono::milliseconds WallTimeElapsed;
    };
    // Indicates that a system error occurred
    // while reading from `nonBlockingFd`
    struct OnSystemError {
        size_t NumBytesRead;
        std::chrono::milliseconds WallTimeElapsed;
        SystemError Err;
    };
    // Indicates that the timeout was reached
    // before the whole buffer `to` was read
    struct OnTimeout {
        size_t NumBytesRead;
        std::chrono::milliseconds WallTimeElapsed;
    };
    // Indicates that an underlying `poll()` syscall returned `POLLERR`
    // or `POLLHUP` in `revents` before the whole buffer `to` was read
    struct OnPollerrOrPollhup {
        size_t NumBytesRead;
        std::chrono::milliseconds WallTimeElapsed;
        int PollRevents;
    };
    using Result = std::variant<
        OnSuccess,
        OnCancellation,
        OnPrematureEof,
        OnSystemError,
        OnTimeout,
        OnPollerrOrPollhup
    >;
}
using RobustSyncReadResult = NRobustSyncRead::Result;
/* Tries to read `to.size()` bytes synchronously from a file descriptor
 * `nonBlockingFd` (which can typically refer to a TCP socket or a pipe) with timeout
 * `timeout` (expressed in milliseconds).
 *
 * The file descriptor `nonBlockingFd` has to be non-blocking
 * (as the name suggests), because `poll` (which is used in the implementation
 * of this function) and `select` syscalls "may report a socket file descriptor
 * as "ready for reading",  while nevertheless  a  subsequent read blocks", as
 * stated in the linux man-page for `select(2)` in the "BUGS" section.
 * Therefore, it is safer to use non-blocking sockets with `poll` and `select`.
 *
 * The optional file descriptor `nonBlockingCancellationFd` enables the read
 * operation to be cancelled from some other place, for example, by the user
 * pressing a "cancel" button in the GUI client interface.
 * If this file descriptor becomes available for reading from, it means
 * that the `read` operation on `nonBlockingFd` has been cancelled.
*/
[[nodiscard]] auto RobustSyncRead(
    int nonBlockingFd,
    std::span<std::byte> to,
    std::chrono::milliseconds timeout,
    std::optional<int> nonBlockingCancellationFd = std::nullopt
) noexcept
  -> RobustSyncReadResult;

namespace NRobustSyncWrite {
    // Indicates that the whole buffer `from` was
    // written to `nonBlockingfd` successfully
    struct OnSuccess {};
    // Indicates that a system error occurred
    // while writing to `nonBlockingFd`
    struct OnSystemError {
        size_t NumBytesWritten;
        SystemError Err;
    };
    // Indicates that the timeout was reached before
    // the whole buffer `from` was written
    struct OnTimeout {
        size_t NumBytesWritten;
        std::chrono::milliseconds Timeout;
    };
    // Indicates that an underlying `poll()` syscall returned `POLLERR` or
    // `POLLHUP` in `revents` before the whole buffer `from` was written
    struct OnPollerrOrPollhup {
        size_t NumBytesWritten;
        int PollRevents;
    };
    using Result = std::variant<OnSuccess, OnSystemError, OnTimeout, OnPollerrOrPollhup>;
}
using RobustSyncWriteResult = NRobustSyncWrite::Result;
[[nodiscard]] auto RobustSyncWrite(
    int nonBlockingFd,
    std::span<const std::byte> from,
    std::chrono::milliseconds timeout,
    std::optional<int> nonBlockingCancellationFd = std::nullopt
) noexcept
  -> RobustSyncWriteResult;
