#include "robust_read_write.hpp"
#include "../../utils/byte_utils.hpp"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <sys/poll.h>
#include <thread>
#include <unistd.h>


namespace {
// A RAII wrapper over a pair of file descriptors
// constituting the read and write ends of a pipe
struct Pipe {
private: 
    int ReadFd;
    int WriteFd;
    Pipe(int pipefds[2]) noexcept : ReadFd(pipefds[0]), WriteFd(pipefds[1]) {}
public:
    Pipe(Pipe&& other) noexcept : ReadFd(other.ReadFd), WriteFd(other.WriteFd) {
        other.ReadFd = -1;
        other.WriteFd = -1;
    }
    static auto CreateNew() noexcept -> std::variant<Pipe, SystemError> {
        int pipefds[2];
        if (pipe2(pipefds, O_NONBLOCK) != 0) {
            return SystemError{
                .Value = std::errc{errno},
                .ContextMessage = "pipe2() syscall failed (" SOURCE_LOCATION ")",
            };
        } else {
            return Pipe{pipefds};
        }
    }
    auto GetReadFd() const noexcept -> int {
        return ReadFd;
    }
    auto GetWriteFd() const noexcept -> int {
        return WriteFd;
    }
    ~Pipe() noexcept {
        if (ReadFd != -1) close(ReadFd);
        if (WriteFd != -1) close(WriteFd);
    }
};

template <std::size_t N>
auto operator==(const std::array<std::byte, N>& lhs, std::string_view rhs) -> bool {
    if (lhs.size() != rhs.size()) return false;
    for (size_t i = 0; i != lhs.size(); ++i) {
        if (lhs[i] != std::byte(rhs[i])) return false;
    }
    return true;
}

using namespace NByteUtils;
} // anonymous namespace


namespace NTests {
using namespace NRobustSyncRead;
static constexpr auto kMessage = std::string_view{"abacaba123"};
using MessageBuf = std::array<std::byte, kMessage.size()>;

struct BasicTestParams {
    std::chrono::milliseconds ReaderTimeout;
    std::chrono::milliseconds WriterSleepTime;
    size_t NBytesForWriterToSend = kMessage.size();
    bool CloseWriterPipe = false;
    std::optional<std::chrono::milliseconds> CancellationTimeout;
};

struct BasicTestResult {
    MessageBuf Data = {};
    std::optional<NRobustSyncRead::Result> ReadResult = std::nullopt;
};

auto RunBasicTest(const BasicTestParams testParams) -> BasicTestResult {
    const auto dataPipe = std::get<Pipe>(Pipe::CreateNew());
    const auto cancellationPipe = std::get<Pipe>(Pipe::CreateNew());
    auto testResult = BasicTestResult{};

    auto reader = std::jthread{[
        &to = testResult.Data,
        &result = testResult.ReadResult,
        fd = dataPipe.GetReadFd(),
        cancellationFd = cancellationPipe.GetReadFd(),
        readerTimeout = testParams.ReaderTimeout
    ]() {
        result = RobustSyncRead(fd, to, readerTimeout, cancellationFd);
    }};

    auto writer = std::jthread([
        fd = dataPipe.GetWriteFd(),
        sleepTime = testParams.WriterSleepTime,
        nBytesToSend = testParams.NBytesForWriterToSend,
        closeFd = testParams.CloseWriterPipe
    ]() {
        for (const auto byte : kMessage.substr(0, nBytesToSend)) {
            std::this_thread::sleep_for(sleepTime);
            assert(write(fd, &byte, sizeof(byte)) == sizeof(byte));
        }
        if (closeFd) close(fd);
    });

    if (testParams.CancellationTimeout) {
        std::this_thread::sleep_for(*testParams.CancellationTimeout);
        const auto byte = std::byte{'x'};
        assert(write(cancellationPipe.GetWriteFd(), &byte, sizeof(byte)) == sizeof(byte));
    }

    return testResult;
}

auto TestEarlyCancellation() -> void {
    const auto testResult = RunBasicTest({
        .ReaderTimeout = std::chrono::milliseconds{1000},
        .WriterSleepTime = std::chrono::milliseconds{100},
        .CancellationTimeout = std::chrono::milliseconds{350},
    });
    assert(testResult.ReadResult.has_value());
    const auto* onCancellationPtr = std::get_if<OnCancellation>(&*testResult.ReadResult);
    assert(onCancellationPtr);
    assert(onCancellationPtr->NumBytesRead == 3);
    const auto expectedData = MessageBuf{'a'_b, 'b'_b, 'a'_b};
    assert(testResult.Data == expectedData);
    std::cerr << "TestEarlyCancellation OK\n";
}

auto TestVeryEarlyCancellation() -> void {
    const auto testResult = RunBasicTest({
        .ReaderTimeout = std::chrono::milliseconds{1000},
        .WriterSleepTime = std::chrono::milliseconds{100},
        .CancellationTimeout = std::chrono::milliseconds{50},
    });
    assert(testResult.ReadResult.has_value());
    const auto* onCancellationPtr = std::get_if<OnCancellation>(&*testResult.ReadResult);
    assert(onCancellationPtr);
    assert(onCancellationPtr->NumBytesRead == 0);
    const auto expectedData = MessageBuf{};
    assert(testResult.Data == expectedData);
    std::cerr << "TestVeryEarlyCancellation OK\n";
}

auto TestTimeout() -> void {
    const auto testResult = RunBasicTest({
        .ReaderTimeout = std::chrono::milliseconds{1000},
        .WriterSleepTime = std::chrono::milliseconds{300},
    });
    assert(testResult.ReadResult.has_value());
    const auto* onTimeoutPtr = std::get_if<OnTimeout>(&*testResult.ReadResult);
    assert(onTimeoutPtr);
    assert(onTimeoutPtr->NumBytesRead == 3);
    const auto expectedData = MessageBuf{'a'_b, 'b'_b, 'a'_b};
    assert(testResult.Data == expectedData);
    std::cerr << "TestTimeout OK\n";
}

auto TestSuccess() -> void {
    const auto testResult = RunBasicTest({
        .ReaderTimeout = std::chrono::milliseconds{1000},
        .WriterSleepTime = std::chrono::milliseconds{90},
    });
    assert(testResult.ReadResult.has_value());
    const auto* onSuccessPtr = std::get_if<OnSuccess>(&*testResult.ReadResult);
    assert(onSuccessPtr);
    assert(onSuccessPtr->NumBytesRead == kMessage.size());
    assert(testResult.Data == kMessage);
    std::cerr << "TestSuccess OK\n";
}

auto TestHangup() -> void {
    const auto testResult = RunBasicTest({
        .ReaderTimeout = std::chrono::milliseconds{1000},
        .WriterSleepTime = std::chrono::milliseconds{100},
        .NBytesForWriterToSend = 5,
        .CloseWriterPipe = true,
    });
    assert(testResult.ReadResult.has_value());
    const auto* onPollerrOrPollhupPtr = std::get_if<OnPollerrOrPollhup>(&*testResult.ReadResult);
    assert(onPollerrOrPollhupPtr);
    assert(onPollerrOrPollhupPtr->NumBytesRead == 5);
    const auto expectedData = MessageBuf{'a'_b, 'b'_b, 'a'_b, 'c'_b, 'a'_b};
    assert(testResult.Data == expectedData);
    assert(onPollerrOrPollhupPtr->PollRevents == POLLHUP);
    std::cerr << "TestHangup OK\n";
}

auto TestNoHangup() -> void {
    const auto testResult = RunBasicTest({
        .ReaderTimeout = std::chrono::milliseconds{1000},
        .WriterSleepTime = std::chrono::milliseconds{90},
        .CloseWriterPipe = true,
    });
    assert(testResult.ReadResult.has_value());
    const auto* onSuccessPtr = std::get_if<OnSuccess>(&*testResult.ReadResult);
    assert(onSuccessPtr);
    assert(onSuccessPtr->NumBytesRead == kMessage.size());
    assert(testResult.Data == kMessage);
    std::cerr << "TestNoHangup OK\n";
}
} // namespace NTests



auto main() -> int {
    using namespace NTests;
    TestEarlyCancellation();
    TestVeryEarlyCancellation();
    TestTimeout();
    TestSuccess();
    TestHangup();
    TestNoHangup();
    std::cerr << "All tests passed.\n";
}
