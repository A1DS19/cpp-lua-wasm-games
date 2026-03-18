#include "../include/main.hpp"

#include <cassert>
#include <iostream>
#include <string_view>

void test_hello() {
    hello();
    std::cout << "hello() test passed" << std::endl;
}

void test_greet() {
    greet("Test");
    std::cout << "greet() test passed" << std::endl;
}

auto main() -> int {
    std::cout << "Running tests..." << std::endl;

    test_hello();
    test_greet();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
