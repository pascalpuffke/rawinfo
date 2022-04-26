#pragma once

#include <fmt/color.h>

#define ALWAYS_INLINE __attribute__((always_inline)) inline

#define ASSERT(expr) ({                                    \
    if (!(expr)) {                                         \
        PANIC(fmt::format("Assertion failed: {}", #expr)); \
    }                                                      \
})

#define ASSERT_MSG(expr, msg) ({                                     \
    if (!(expr)) {                                                   \
        PANIC(fmt::format("Assertion failed: {} ({})", #expr, msg)); \
    }                                                                \
})

#define PANIC(msg) ({                                                                                        \
    fmt::print(stderr, fmt::fg(fmt::color::red), "{}:{}:{} panic: {}\n", __FILE__, __LINE__, __func__, msg); \
    std::abort();                                                                                            \
})

#define UNREACHABLE() ({       \
    PANIC("Unreachable code"); \
})

#define ITERATORS(container) container.begin(), container.end()
