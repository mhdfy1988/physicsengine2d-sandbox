#ifndef PHYSICS_FILESYSTEM_HPP
#define PHYSICS_FILESYSTEM_HPP

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace physics2d {
namespace foundation {

using Path = std::filesystem::path;

struct FileInfo {
    bool exists = false;
    bool is_directory = false;
    std::uintmax_t size_bytes = 0;
};

inline Path normalize_path(const Path& path) {
    std::error_code ec;
    Path normalized = path.lexically_normal();
    if (normalized.is_absolute()) {
        return normalized;
    }
    Path absolute = std::filesystem::absolute(normalized, ec);
    if (!ec) {
        return absolute.lexically_normal();
    }
    return normalized;
}

inline bool ensure_directory(const Path& path) {
    std::error_code ec;
    if (path.empty()) {
        return false;
    }
    if (std::filesystem::exists(path, ec)) {
        return !ec && std::filesystem::is_directory(path, ec);
    }
    return std::filesystem::create_directories(path, ec) || (!ec && std::filesystem::exists(path, ec));
}

inline bool remove_if_exists(const Path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return true;
    }
    (void)std::filesystem::remove_all(path, ec);
    return !ec;
}

inline bool write_text_file(const Path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    return out.good();
}

inline bool read_text_file(const Path& path, std::string& out_text) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream buffer;
    if (!in.is_open()) {
        return false;
    }
    buffer << in.rdbuf();
    if (!in.good() && !in.eof()) {
        return false;
    }
    out_text = buffer.str();
    return true;
}

inline FileInfo stat_file(const Path& path) {
    std::error_code ec;
    FileInfo info;
    info.exists = std::filesystem::exists(path, ec);
    if (ec || !info.exists) {
        return info;
    }
    info.is_directory = std::filesystem::is_directory(path, ec);
    if (ec) {
        info.is_directory = false;
        return info;
    }
    if (!info.is_directory) {
        info.size_bytes = std::filesystem::file_size(path, ec);
        if (ec) {
            info.size_bytes = 0;
        }
    }
    return info;
}

}  // namespace foundation
}  // namespace physics2d

#endif
