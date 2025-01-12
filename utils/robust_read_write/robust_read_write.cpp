#include "robust_read_write.hpp"

#include "../../utils/timer/timer.hpp"

#include <cerrno>
#include <chrono>
#include <sys/poll.h>
#include <unistd.h>


auto RobustSyncRead(
    const int fd,
    const std::span<std::byte> to,
    const std::chrono::milliseconds timeout
) noexcept
  -> RobustSyncReadResult {
    using namespace NRobustSyncRead;
    auto pollFd = pollfd{
        .fd = fd,
        .events = POLLIN,
        .revents = 0,
    };
    const auto timer = Timer{timeout};
    auto nBytesRead = size_t{0};
    auto remainingTime = timeout;
    for (; nBytesRead != to.size(); remainingTime = timer.CalcRemainingTime()) {
        const auto pollResult = poll(&pollFd, 1, remainingTime.count());
        if (pollResult == -1) {
            if (errno == EINTR) continue;
            else return OnSystemError{
                .NumBytesRead = nBytesRead,
                .Err = SystemError{
                    .Value = std::errc{errno},
                    .ContextMessage = "poll() syscall failed (" SOURCE_LOCATION ")",
                },
            };
        } else if (pollResult == 0) {
            return OnTimeout{
                .NumBytesRead = nBytesRead,
                .Timeout = timeout,
            };
        } else { // pollResult == 1
            if (pollFd.revents & POLLIN) {  
                const auto x = read(fd, &to[nBytesRead], to.size() - nBytesRead);
                if (x == -1) {
                    // Check for `EAGAIN` and `EWOULDBLOCK` in case `poll` spuriously reported
                    // that input on `nonBlockingFd` is available
                    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                    else return OnSystemError{
                        .NumBytesRead = nBytesRead,
                        .Err = SystemError{
                            .Value = std::errc{errno},
                            .ContextMessage = "read() syscall failed (" SOURCE_LOCATION ")",
                        },
                    };
                } else if (x == 0) { // no more data available for reading
                    // Note: at this point the loop invariant is holding:
                    // `nBytesRead` is strictly less than `to.size()`
                    return OnPrematureEof{
                        .NumBytesRead = nBytesRead,
                    };
                } else {
                    nBytesRead += x;
                }
            } else { // `POLLERR | POLLHUP | POLLNVAL`
                // See https://stackoverflow.com/questions/24791625/how-to-handle-the-linux-socket-revents-pollerr-pollhup-and-pollnval
                if (pollFd.revents == POLLNVAL) {
                    return OnSystemError{
                        .NumBytesRead = nBytesRead,
                        .Err = SystemError{
                            .Value = std::errc{EBADF},
                            .ContextMessage = "poll() syscall failed (" SOURCE_LOCATION "), "
                                              "the file descriptor "
                                              + std::to_string(fd)
                                              + " was not open"
                        }
                    };
                } else {
                    return OnPollerrOrPollhup{
                        .NumBytesRead = nBytesRead,
                        .PollRevents = pollFd.revents,
                    };
                }
            }
        }
    }
    return OnSuccess{};
}

auto RobustSyncWrite(
    const int fd,
    const std::span<const std::byte> from,
    const std::chrono::milliseconds timeout
) noexcept
  -> RobustSyncWriteResult {
    using namespace NRobustSyncWrite;
    auto pollFd = pollfd{
        .fd = fd,
        .events = POLLOUT,
        .revents = 0,
    };
    const auto timer = Timer{timeout};
    auto nBytesWritten = size_t{0};
    auto remainingTime = timeout;
    for (; nBytesWritten != from.size(); remainingTime = timer.CalcRemainingTime()) {
        const auto pollResult = poll(&pollFd, 1, remainingTime.count());
        if (pollResult == -1) {
            if (errno == EINTR) continue;
            else return OnSystemError{
                .NumBytesWritten = nBytesWritten,
                .Err = SystemError{
                    .Value = std::errc{errno},
                    .ContextMessage = "poll() syscall failed (" SOURCE_LOCATION ")",
                },
            };
        } else if (pollResult == 0) {
            return OnTimeout{
                .NumBytesWritten = nBytesWritten,
                .Timeout = timeout,
            };
        } else { // pollResult == 1
            if (pollFd.revents & POLLOUT) {
                const auto x = write(fd, &from[nBytesWritten], from.size() - nBytesWritten);
                if (x == -1) {
                    // Check for `EAGAIN` and `EWOULDBLOCK` in case `poll` spuriously reported
                    // that output on `fd` is available or the number of bytes that it is possible
                    // to output to `fd` is smaller than `from.size() - nBytesWritten`
                    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                    else return OnSystemError{
                        .NumBytesWritten = nBytesWritten,
                        .Err = SystemError{
                            .Value = std::errc{errno},
                            .ContextMessage = "write() syscall failed (" SOURCE_LOCATION ")",
                        },
                    };
                } else { // `x` > 0 because `write()` can't return 0 when writing more than 0 bytes
                    nBytesWritten += x;
                }
            } else { // `POLLERR | POLLHUP | POLLNVAL`
                if (pollFd.revents == POLLNVAL) {
                    return OnSystemError{
                        .NumBytesWritten = nBytesWritten,
                        .Err = SystemError{
                            .Value = std::errc{EBADF},
                            .ContextMessage = "poll() syscall failed (" SOURCE_LOCATION "), "
                                              "the file descriptor "
                                              + std::to_string(fd)
                                              + " was not open"
                        }
                    };
                } else {
                    return OnPollerrOrPollhup{
                        .NumBytesWritten = nBytesWritten,
                        .PollRevents = pollFd.revents,
                    };
                }
            }
        }
    }
    return OnSuccess{};
}