#pragma once

#include "em/macros/utils/functors.h"
#include "em/macros/utils/returns.h"
#include "em/math/apply_elementwise.h"
#include "em/math/larger_type.h"
#include "em/math/scalar.h"
#include "em/meta/compare.h"

#include <cmath>
#include <compare>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <type_traits>

// This header is used to actively check for narrowing conversions/overflows/etc and catch them.
// About reliability:
// The `float x float` comparisons should be completely reliable, as we simply use the builtin comparison operators.
// The `int x int` comparisons should also be reliable, as the comparison algorithm is simple.
// The `int x float` comparisons, on the other hand, rely on a complicated algorithm. Even though they were tested,
//     it's hard to guarantee complete robustness here. Also they might be slow.

namespace em::Math::Robust
{
    template <typename T>
    concept builtin_scalar = std::is_arithmetic_v<T>;

    namespace detail
    {
        // Compares an integral and a floating-point value.
        // Despite the parameter names, it doesn't matter which one is which.
        // Follows a so-called 'partial ordering': for some pairs of values you get a special 'undefined' result (i.e. for NaNs compared with any number).
        // See following thread for the explanation of the algorithm and for alternative implementations:
        //   https://stackoverflow.com/questions/58734034/how-to-properly-compare-an-integer-and-a-floating-point-value
        template <builtin_scalar I, builtin_scalar F>
        [[nodiscard]] constexpr std::partial_ordering compare_int_float_three_way(I i, F f) noexcept
        {
            if constexpr (std::is_integral_v<F> && std::is_floating_point_v<I>)
            {
                return 0 <=> compare_int_float_three_way(f, i);
            }
            else
            {
                static_assert(std::is_integral_v<I> && std::is_floating_point_v<F>);
                static_assert(std::numeric_limits<F>::radix == 2);

                // This should be exactly representable as F due to being a power of two.
                constexpr F I_min_as_F = std::numeric_limits<I>::min();

                // The `numeric_limits<I>::max()` itself might not be representable as F, so we use this instead.
                constexpr F I_max_as_F_plus_1 = F(std::numeric_limits<I>::max()/2+1) * 2;

                // Check if the constants above overflowed to infinity. Normally this shouldn't happen.
                constexpr bool limits_overflow = I_min_as_F * 2 == I_min_as_F || I_max_as_F_plus_1 * 2 == I_max_as_F_plus_1;
                if constexpr (limits_overflow)
                {
                    // Manually check for special floating-point values.
                    if (std::isinf(f))
                        return f > 0 ? std::partial_ordering::less : std::partial_ordering::greater;
                    if (std::isnan(f))
                        return std::partial_ordering::unordered;
                }

                if (limits_overflow || f >= I_min_as_F)
                {
                    // `f <= I_max_as_F_plus_1 - 1` would be problematic due to rounding, so we use this instead.
                    if (limits_overflow || f - I_max_as_F_plus_1 <= -1)
                    {
                        I f_trunc = I(f);
                        if (f_trunc < i)
                            return std::partial_ordering::greater;
                        if (f_trunc > i)
                            return std::partial_ordering::less;

                        F f_frac = f - f_trunc;
                        if (f_frac < 0)
                            return std::partial_ordering::greater;
                        if (f_frac > 0)
                            return std::partial_ordering::less;

                        return std::partial_ordering::equivalent;
                    }

                    return std::partial_ordering::less;
                }

                if (f < 0)
                    return std::partial_ordering::greater;

                return std::partial_ordering::unordered;
            }
        }

        template <std::integral A, std::integral B>
        [[nodiscard]] constexpr std::strong_ordering compare_integers_three_way(A a, B b) noexcept
        {
            if constexpr (std::is_signed_v<A> == std::is_signed_v<B>)
            {
                return a <=> b;
            }
            else if constexpr (std::is_signed_v<B>)
            {
                // Flip.
                return 0 <=> (compare_integers_three_way)(b, a);
            }
            else
            {
                // Here `A` is signed and `B` is unsigned.

                if (a < 0)
                    return std::strong_ordering::less;

                // This picks the larger of two types. If they have the same size but different signedness, the unsigned one is used.
                using C = std::common_type_t<A, B>;

                return C(a) <=> C(b);
            }
        }
    }

    // For internal use, and also a customization point. Prefer the public functions below.
    template <builtin_scalar A, builtin_scalar B>
    [[nodiscard]] constexpr auto _adl_em_robust_compare_scalars_three_way(A a, B b) noexcept
    {
        if constexpr (std::is_floating_point_v<A> && std::is_floating_point_v<B>)
        {
            return a <=> b;
        }
        else if constexpr (!std::is_floating_point_v<A> && !std::is_floating_point_v<B>)
        {
            return detail::compare_integers_three_way(a, b);
        }
        else
        {
            return detail::compare_int_float_three_way(a, b);
        }
    }

    // A separate customization point for equality, if needed.
    [[nodiscard]] constexpr auto _adl_em_robust_compare_scalars_equal(const auto &a, const auto &b) EM_RETURNS(_adl_em_robust_compare_scalars_three_way(a, b) == 0)


    namespace detail
    {
        // Wraps `_adl_em_robust_compare_scalars_three_way()` to wrap the result in `Meta::BoolTestableCompareResult`, and to make it a functor.
        EM_SIMPLE_FUNCTOR( CompareThreeWayBoolTestable,, (const auto &a, const auto &b) EM_RETURNS(Meta::BoolTestableCompareResult(_adl_em_robust_compare_scalars_three_way(a, b))) )
    }


    // Now the public functions:

    // Compares scalars/vectors/etc for equality, correctly handling the type differences.
    EM_SIMPLE_ALL_OF_FUNCTOR( equal,, (const auto &a, const auto &b) EM_RETURNS(_adl_em_robust_compare_scalars_equal(a, b)) )
    EM_SIMPLE_FUNCTOR( not_equal,, (const auto &a, const auto &b) EM_RETURNS(!equal(a, b)) )

    EM_SIMPLE_FUNCTOR( compare_three_way,, (const auto &a, const auto &b) EM_RETURNS(auto((apply_elementwise)(detail::CompareThreeWayBoolTestable, a, b).value)) )

    EM_SIMPLE_FUNCTOR( less         ,, (const auto &a, const auto &b) EM_RETURNS(compare_three_way(a, b) <  0) )
    EM_SIMPLE_FUNCTOR( greater      ,, (const auto &a, const auto &b) EM_RETURNS(compare_three_way(a, b) >  0) )
    EM_SIMPLE_FUNCTOR( less_equal   ,, (const auto &a, const auto &b) EM_RETURNS(compare_three_way(a, b) <= 0) )
    EM_SIMPLE_FUNCTOR( greater_equal,, (const auto &a, const auto &b) EM_RETURNS(compare_three_way(a, b) >= 0) )

    // A helper to quickly peform comparisons.
    // For example: `compare(x) <= x` expands to `Robust::less_eq(x, y)`.
    template <typename T>
    class [[nodiscard]] compare
    {
        const T &value;

      public:
        constexpr compare(const T &value) : value(value) {}
        compare(const compare &) = delete;
        compare &operator=(const compare &) = delete;

        // Those are automatically symmetric in C++20.
        // Not `friend` because I don't want any conversions in the lhs.
        [[nodiscard]] constexpr auto operator==(const auto &other) EM_RETURNS(equal(value, other))
        [[nodiscard]] constexpr auto operator<=>(const auto &other) EM_RETURNS(compare_three_way(value, other))
    };


    // Returns true if `value` can be represented as `A`.
    // Usage: `representable_as<TargetType>(value)`.
    //
    // The warning has to be silenced outside of the macro, for some reason. The warning happens e.g. `representable_as<ivec2>(1.2f)`.
    EM_SILENCE_DIAGNOSTIC_IF(EM_IS_GCC_LIKE, "-Wfloat-conversion")(
    EM_SIMPLE_FUNCTOR_EXT( representable_as,
        (template <typename To>), (EM_1<To>),
        (template <typename From> requires std::constructible_from<To, const From &>), // Using "constructible" to allow scalar-to-vector stuff.
        (const From &value) EM_RETURNS(equal(To(value), value))
    )
    )


    // Attempts to convert `value` to type `A`. Throws if it's not represesntable as `A`.
    // Usage `Robust::cast<A>(value)`.
    EM_SIMPLE_FUNCTOR_EXT( cast,
        (template <typename To>), (EM_1<To>),
        (template <typename From> requires std::constructible_from<To, const From &>),
        (const From &value)
        {
            To result(value);
            if (not_equal(result, value))
                throw std::runtime_error("Narrowing cast failed.");
            return result;
        }
    )
}
