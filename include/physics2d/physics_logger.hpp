#ifndef PHYSICS_LOGGER_HPP
#define PHYSICS_LOGGER_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace physics2d {
namespace foundation {

enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

inline const char* log_level_name(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug: return "debug";
        case LogLevel::Info: return "info";
        case LogLevel::Warn: return "warn";
        case LogLevel::Error: return "error";
        case LogLevel::Fatal: return "fatal";
    }
    return "info";
}

struct LogRecord {
    LogLevel level = LogLevel::Info;
    std::string category;
    std::string message;
};

using LogSink = std::function<void(const LogRecord&)>;

class Logger {
public:
    void add_sink(LogSink sink) {
        sinks_.push_back(std::move(sink));
    }

    std::size_t sink_count() const noexcept {
        return sinks_.size();
    }

    void log(LogLevel level, std::string category, std::string message) const {
        LogRecord record;
        record.level = level;
        record.category = std::move(category);
        record.message = std::move(message);
        for (const LogSink& sink : sinks_) {
            sink(record);
        }
    }

private:
    std::vector<LogSink> sinks_;
};

}  // namespace foundation
}  // namespace physics2d

#endif
