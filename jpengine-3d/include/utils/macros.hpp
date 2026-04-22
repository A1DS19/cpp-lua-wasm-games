#pragma once

namespace utils::macros {
[[nodiscard]] constexpr bool convert_to_bool(auto value) noexcept {
    return static_cast<bool>(value);
}
} // namespace utils::macros
