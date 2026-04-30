#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#include "types.h"

namespace fs {
    inline std::optional<std::vector<u8>> loadFileIntoBuffer(const std::filesystem::path& filepath) {
        std::ifstream file {filepath, std::ios::binary};
        if (!file) return std::nullopt;

        // Get filesize
        file.seekg(0, std::ios::end);
        auto fileSize = file.tellg();
        if (fileSize < 0) return std::nullopt;
        file.seekg(0, std::ios::beg);

        std::vector<u8> buffer(fileSize);

        // Copy file data to buffer
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        if (!file) return std::nullopt;

        return buffer;
    }
}