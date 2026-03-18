#include "main.hpp"

#include <cstdlib>
#include <format> // C++23 feature
#include <iostream>
#include <string_view>

void hello() {
    std::cout << "Hello, World!" << std::endl;
}

void greet(std::string_view name) {
// Using C++23 std::format if available, fallback to iostream
#ifdef __cpp_lib_format
    std::cout << std::format("Hello, {}!\n", name);
#else
    std::cout << "Hello, " << name << "!" << std::endl;
#endif
}

auto main() -> int {
    hello();
    greet("Modern C++");
    return EXIT_SUCCESS;
}
