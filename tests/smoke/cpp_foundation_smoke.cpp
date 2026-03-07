#include <atomic>
#include <cstdio>
#include <string>
#include <vector>
#include "../../include/physics2d/physics_foundation.hpp"

int main() {
    using namespace physics2d::foundation;

    const Path root = "_tmp_cpp_foundation_project";
    const RuntimePathConfig config = RuntimePathConfig::from_project_root(root);
    std::string file_text;
    std::vector<LogRecord> records;
    std::atomic<int> total{0};
    TaskRunner runner(4);

    if (!config.valid() || !config.ensure_minimal_tree()) {
        std::printf("[FAIL] foundation config tree init failed\n");
        return 1;
    }

    if (!write_text_file(config.assets_dir / "note.txt", "foundation-smoke") ||
        !read_text_file(config.assets_dir / "note.txt", file_text) ||
        file_text != "foundation-smoke") {
        std::printf("[FAIL] foundation filesystem read/write mismatch\n");
        remove_if_exists(config.project_root);
        return 2;
    }

    const FileInfo info = stat_file(config.assets_dir / "note.txt");
    if (!info.exists || info.is_directory || info.size_bytes == 0) {
        std::printf("[FAIL] foundation filesystem stat mismatch\n");
        remove_if_exists(config.project_root);
        return 3;
    }

    Logger logger;
    logger.add_sink([&records](const LogRecord& record) {
        records.push_back(record);
    });
    logger.log(LogLevel::Info, "phase_e", "foundation-online");
    logger.log(LogLevel::Warn, "phase_e", "foundation-warning");
    if (logger.sink_count() != 1u ||
        records.size() != 2u ||
        records[0].category != "phase_e" ||
        records[1].message != "foundation-warning" ||
        std::string(log_level_name(records[1].level)) != "warn") {
        std::printf("[FAIL] foundation logger mismatch\n");
        remove_if_exists(config.project_root);
        return 4;
    }

    runner.parallel_for(128, 8, [&total](int begin, int end) {
        int local = 0;
        for (int i = begin; i < end; ++i) {
            local += 1;
        }
        total.fetch_add(local, std::memory_order_relaxed);
    });
    if (runner.worker_count() < 1 || total.load(std::memory_order_relaxed) != 128) {
        std::printf("[FAIL] foundation task runner mismatch\n");
        remove_if_exists(config.project_root);
        return 5;
    }

    if (!remove_if_exists(config.project_root)) {
        std::printf("[FAIL] foundation cleanup failed\n");
        return 6;
    }

    std::printf("[PASS] cpp foundation smoke\n");
    return 0;
}
