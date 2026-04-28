#include "utils/file-utils.hpp"

#include "utils/asset-path.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace utils {

std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        throw std::runtime_error("failed to open file: " + path.string());
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

std::string read_asset_text(std::string_view relative_path) {
    return read_text_file(asset_path(relative_path));
}

} // namespace utils
