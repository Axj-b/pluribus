#include "test.h"

#include <exception>
#include <iostream>

int main() {
    std::size_t passed = 0;
    std::size_t failed = 0;

    for (const auto& test : pluribus::test::registry()) {
        try {
            test.function();
            ++passed;
        } catch (const pluribus::test::Failure& failure) {
            ++failed;
            std::cerr << "[FAIL] " << test.name << "\n  " << failure.what() << '\n';
        } catch (const std::exception& exception) {
            ++failed;
            std::cerr << "[FAIL] " << test.name << "\n  unexpected exception: " << exception.what()
                      << '\n';
        } catch (...) {
            ++failed;
            std::cerr << "[FAIL] " << test.name << "\n  unexpected non-standard exception\n";
        }
    }

    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
