#pragma once

#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace pluribus::test {

struct Failure : std::exception {
    explicit Failure(std::string text) : message(std::move(text)) {}

    const char* what() const noexcept override { return message.c_str(); }

    std::string message;
};

struct TestCase {
    const char* name;
    void (*function)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const char* name, void (*function)()) { registry().push_back({name, function}); }
};

inline void check(
    bool condition,
    const char* expression,
    const char* file,
    int line) {
    if (!condition) {
        throw Failure(
            std::string(file) + ":" + std::to_string(line) + ": CHECK failed: " + expression);
    }
}

} // namespace pluribus::test

#define PLURIBUS_TEST(name)                                                  \
    static void name();                                                      \
    static const ::pluribus::test::Registrar name##_registrar(#name, &name); \
    static void name()

#define CHECK(expression) \
    ::pluribus::test::check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
