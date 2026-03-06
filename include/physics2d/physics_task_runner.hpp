#ifndef PHYSICS_TASK_RUNNER_HPP
#define PHYSICS_TASK_RUNNER_HPP

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

namespace physics2d {
namespace foundation {

class TaskRunner {
public:
    explicit TaskRunner(int worker_count = 0) noexcept
        : worker_count_(worker_count > 0 ? worker_count : default_worker_count()) {}

    int worker_count() const noexcept {
        return worker_count_;
    }

    template <typename Fn>
    void parallel_for(int count, int grain, Fn fn) const {
        if (count <= 0) {
            return;
        }
        if (grain <= 0) {
            grain = count;
        }
        const int chunk_count = (count + grain - 1) / grain;
        const int workers = std::min(worker_count_, chunk_count);
        if (workers <= 1) {
            for (int begin = 0; begin < count; begin += grain) {
                const int end = std::min(begin + grain, count);
                fn(begin, end);
            }
            return;
        }

        std::vector<std::thread> threads;
        threads.reserve(static_cast<std::size_t>(workers - 1));
        std::atomic<int> next_chunk{0};
        auto run = [&]() {
            for (;;) {
                const int chunk_index = next_chunk.fetch_add(1, std::memory_order_relaxed);
                int begin;
                int end;
                if (chunk_index >= chunk_count) {
                    return;
                }
                begin = chunk_index * grain;
                end = std::min(begin + grain, count);
                fn(begin, end);
            }
        };

        for (int i = 1; i < workers; ++i) {
            threads.emplace_back(run);
        }
        run();
        for (std::thread& thread : threads) {
            thread.join();
        }
    }

private:
    static int default_worker_count() noexcept {
        const unsigned int hc = std::thread::hardware_concurrency();
        return hc > 0 ? static_cast<int>(hc) : 1;
    }

    int worker_count_ = 1;
};

}  // namespace foundation
}  // namespace physics2d

#endif
