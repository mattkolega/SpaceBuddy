#include "fs.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "log.h"
#include "types.h"

std::optional<std::vector<u8>> fs::loadFileIntoBuffer(const std::filesystem::path &filepath) {
    std::ifstream file {filepath, std::ios::binary};
    if (!file) {
        log::fatal("Failed to open file: `{}` Error: {}", filepath, std::strerror(errno));
        return std::nullopt;
    }

    // Get filesize
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    if (fileSize < 0) {
        log::fatal("Failed to open file: `{}` Error: Invalid file size", filepath);
        return std::nullopt;
    }
    file.seekg(0, std::ios::beg);

    std::vector<u8> buffer(fileSize);

    // Copy file data to buffer
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    if (!file) {
        log::fatal("Failed to open file: `{}` Error: {}", filepath, std::strerror(errno));
        return std::nullopt;
    }

    return buffer;
}

std::filesystem::path fs::getExeDir() {
    std::error_code ec;
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);

    auto exePath = std::filesystem::canonical(buf, ec)
    if (!ec) return exePath.parent_path();
#elifdef __APPLE__
    uint32_t size{0};
    _NSGetExecutablePath(nullptr, &size);
    std::string buf(size, '\0');
    _NSGetExecutablePath(buf.data(), &size);

    auto exePath = std::filesystem::canonical(buf, ec);
    if (!ec) return exePath.parent_path();
#elifdef __linux__
    auto exePath = std::filesystem::canonical("/proc/self/exe", ec)
    if (!ec) return exePath.parent_path();
#else
    log::warning("getExeDir: unsupported platform, returning empty path.")
#endif
    return "";
}
