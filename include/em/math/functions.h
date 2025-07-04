#pragma once

#include "em/macros/utils/returns.h"
#include "em/math/apply_elementwise.h"
#include "em/math/larger_type.h"
#include "em/math/namespaces.h"
#include "em/math/scalar.h"

namespace em::Math
{
    // Changes the type of the parameter to something floating-point (a scalar or a vector).
    // If it's not already floating-point, it becomes `float` or a vector of it.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( make_floating_point,, (const auto &a) EM_RETURNS(floating_point_t<decltype(a)>(a)) )

    // Pi.
    template <typename T> constexpr T pi = T(3.14159265358979323846l);
    constexpr float       f_pi  = pi<float>;
    constexpr double      d_pi  = pi<double>;
    constexpr long double ld_pi = pi<long double>;

    // Convert between degrees and radians.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( deg_to_rad,
        (template <scalar T, typename F = floating_point_t<T>>),
        (T a) EM_RETURNS(F(a) * pi<F> / F(180))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( rad_to_deg,
        (template <scalar T, typename F = floating_point_t<T>>),
        (T a) EM_RETURNS(F(a) * F(180) / pi<F>)
    )


    // The sign of `a` as an `int`.
    // This works returns with vectors directly, but we still add elementwise-ness for consistency. Our macros should call this directly anyway,
    //   without resorting to elementwise if it isn't needed.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( sign,, (const auto &a) EM_RETURNS((a > 0) - (a < 0)) )

    // The sign of `a - b` as an `int`.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( diffsign,, (const auto &a, const auto &b) EM_RETURNS((a > b) - (a < b)) )


    // Clamp a value and return a clamped copy. The original is unchanged.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_low,
        (template <typename T, typename A, typename L = larger_t<T, A>>),
        (const T &target, const A &low) EM_RETURNS(target >= low ? L(target) : L(low))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_high,
        (template <typename T, typename A, typename L = larger_t<T, A>>),
        (const T &target, const A &high) EM_RETURNS(target <= high ? L(target) : L(high))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp,
        (template <typename T, typename A, typename B, typename L = larger_t<T, A, B>>),
        (const T &target, const A &low, const B &high) EM_RETURNS(target >= low ? (target <= high ? L(target) : L(high)) : L(low))
    )


    // Takes a variable by reference and clamps it from one or both sides.
    // Note that NaN gets replaced with the bound, so we need to be careful about how we compare things.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_low,
        (template <typename T, can_safely_convert_to<T> A>),
        (T &target, const A &low) EM_RETURNS(target >= low ? void() : void(target = low))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_high,
        (template <typename T, can_safely_convert_to<T> A>),
        (T &target, const A &high) EM_RETURNS(target <= high ? void() : void(target = high))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var,
        (template <typename T, can_safely_convert_to<T> A, can_safely_convert_to<T> B>),
        // This isn't implemented in terms of `clamp_var_{high,low}` to better match how `clamp()` behaves with inverted bounds and NaN.
        (T &target, const A &low, const B &high) EM_RETURNS(target >= low ? (target <= high ? void() : void(target = high)) : void(target = low))
    )


    inline namespace Common
    {
        using Math::make_floating_point;
        using Math::pi, Math::f_pi, Math::d_pi, Math::ld_pi;
        using Math::deg_to_rad;
        using Math::rad_to_deg;
        using Math::sign;
        using Math::diffsign;
        using Math::clamp_var_low;
        using Math::clamp_var_high;
        using Math::clamp_var;
        using Math::clamp_low;
        using Math::clamp_high;
        using Math::clamp;
    }
}
