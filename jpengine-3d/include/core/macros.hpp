#pragma once

namespace core::macros {
[[nodiscard]] constexpr bool convert_to_bool(auto value) noexcept {
    return static_cast<bool>(value);
}
} // namespace core::macros
