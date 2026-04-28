#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace utils {

// Read the entire contents of a text file.
// Throws std::runtime_error if the file cannot be opened.
[[nodiscard]] std::string read_text_file(const std::filesystem::path& path);

// Read a text file relative to the project's assets/ directory.
//   read_asset_text("shaders/vertex.glsl")
// Throws std::runtime_error if the file cannot be opened.
[[nodiscard]] std::string read_asset_text(std::string_view relative_path);

} // namespace utils
