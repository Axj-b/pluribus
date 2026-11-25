#pragma once

#include <cstddef>
#include <cstdint>

namespace pluribus::scheduler::detail {

// Small, dependency-free wide integer helpers used for exact scheduler
// arithmetic. Only the operations required by the V1 policy are provided.

struct UInt128 {
    std::uint64_t high = 0;
    std::uint64_t low = 0;
};

inline UInt128 from_u64(std::uint64_t value) noexcept {
    return {0, value};
}

inline int compare(UInt128 left, UInt128 right) noexcept {
    if (left.high != right.high) {
        return left.high < right.high ? -1 : 1;
    }
    if (left.low != right.low) {
        return left.low < right.low ? -1 : 1;
    }
    return 0;
}

inline UInt128 multiply_u64(std::uint64_t left, std::uint64_t right) noexcept {
    const std::uint64_t left_low = left & 0xFFFF'FFFFULL;
    const std::uint64_t left_high = left >> 32;
    const std::uint64_t right_low = right & 0xFFFF'FFFFULL;
    const std::uint64_t right_high = right >> 32;

    const std::uint64_t p0 = left_low * right_low;
    const std::uint64_t p1 = left_low * right_high;
    const std::uint64_t p2 = left_high * right_low;
    const std::uint64_t p3 = left_high * right_high;

    const std::uint64_t middle1 = p1 + (p0 >> 32);
    const std::uint64_t middle2 = p2 + (middle1 & 0xFFFF'FFFFULL);
    const std::uint64_t high = p3 + (middle1 >> 32) + (middle2 >> 32);
    const std::uint64_t low = (middle2 << 32) | (p0 & 0xFFFF'FFFFULL);
    return {high, low};
}

inline bool add(UInt128 left, UInt128 right, UInt128& result) noexcept {
    const std::uint64_t low = left.low + right.low;
    const std::uint64_t low_carry = low < left.low ? 1ULL : 0ULL;

    const std::uint64_t high_without_carry = left.high + right.high;
    const bool high_overflow = high_without_carry < left.high;
    const std::uint64_t high = high_without_carry + low_carry;
    const bool carry_overflow = high < high_without_carry;

    if (high_overflow || carry_overflow) {
        return false;
    }
    result = {high, low};
    return true;
}

inline bool multiply(UInt128 value, std::uint64_t factor, UInt128& result) noexcept {
    const UInt128 low_part = multiply_u64(value.low, factor);
    const UInt128 high_part = multiply_u64(value.high, factor);

    const std::uint64_t high = high_part.low + low_part.high;
    const bool overflow = high < high_part.low;
    if (high_part.high != 0 || overflow) {
        return false;
    }
    result = {high, low_part.low};
    return true;
}

struct UInt256 {
    std::uint64_t limbs[4]{};
};

inline void add_limb(UInt256& value, std::size_t index, std::uint64_t addend) noexcept {
    std::uint64_t carry = addend;
    for (std::size_t limb = index; limb < 4 && carry != 0; ++limb) {
        const std::uint64_t previous = value.limbs[limb];
        value.limbs[limb] = previous + carry;
        carry = value.limbs[limb] < previous ? 1ULL : 0ULL;
    }
}

inline UInt256 multiply(UInt128 left, UInt128 right) noexcept {
    const UInt128 p00 = multiply_u64(left.low, right.low);
    const UInt128 p01 = multiply_u64(left.low, right.high);
    const UInt128 p10 = multiply_u64(left.high, right.low);
    const UInt128 p11 = multiply_u64(left.high, right.high);

    UInt256 result;
    result.limbs[0] = p00.low;
    add_limb(result, 1, p00.high);
    add_limb(result, 1, p01.low);
    add_limb(result, 1, p10.low);
    add_limb(result, 2, p01.high);
    add_limb(result, 2, p10.high);
    add_limb(result, 2, p11.low);
    add_limb(result, 3, p11.high);
    return result;
}

inline int compare(UInt256 left, UInt256 right) noexcept {
    for (std::size_t index = 4; index-- > 0;) {
        if (left.limbs[index] != right.limbs[index]) {
            return left.limbs[index] < right.limbs[index] ? -1 : 1;
        }
    }
    return 0;
}

} // namespace pluribus::scheduler::detail
