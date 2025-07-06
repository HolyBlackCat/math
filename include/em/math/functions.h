#pragma once

#include "em/macros/utils/returns.h"
#include "em/math/apply_elementwise.h"
#include "em/math/larger_type.h"
#include "em/math/namespaces.h"
#include "em/math/scalar.h"

#include <cmath>

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
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( sign,, (const scalar auto &a) EM_RETURNS((a > 0) - (a < 0)) )

    // The sign of `a - b` as an `int`.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( diffsign,, (const scalar auto &a, const scalar auto &b) EM_RETURNS((a > b) - (a < b)) )


    // Clamp a value and return a clamped copy. The original is unchanged.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_low,
        (template <scalar T, scalar A, typename L = larger_t<T, A>>),
        (const T &target, const A &low) EM_RETURNS(target >= low ? L(target) : L(low))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_high,
        (template <scalar T, scalar A, typename L = larger_t<T, A>>),
        (const T &target, const A &high) EM_RETURNS(target <= high ? L(target) : L(high))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp,
        (template <scalar T, scalar A, scalar B, typename L = larger_t<T, A, B>>),
        (const T &target, const A &low, const B &high) EM_RETURNS(target >= low ? (target <= high ? L(target) : L(high)) : L(low))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_abs,,
        (const auto &target, const auto &abs_limit) EM_RETURNS(clamp(target, -abs_limit, abs_limit))
    )


    // Takes a variable by reference and clamps it from one or both sides.
    // Note that NaN gets replaced with the bound, so we need to be careful about how we compare things.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_low,
        (template <scalar T, scalar A> requires can_safely_convert_to<A, T>),
        (T &target, const A &low) EM_RETURNS(target >= low ? void() : void(target = low))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_high,
        (template <scalar T, scalar A> requires can_safely_convert_to<A, T>),
        (T &target, const A &high) EM_RETURNS(target <= high ? void() : void(target = high))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var,
        (template <scalar T, scalar A, scalar B> requires can_safely_convert_to<A, T> && can_safely_convert_to<B, T>),
        // This isn't implemented in terms of `clamp_var_{high,low}` to better match how `clamp()` behaves with inverted bounds and NaN.
        (T &target, const A &low, const B &high) EM_RETURNS(target >= low ? (target <= high ? void() : void(target = high)) : void(target = low))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_abs,,
        (scalar auto &target, const scalar auto &abs_limit) EM_RETURNS(clamp_var(target, -abs_limit, abs_limit))
    )


    namespace detail::Funcs
    {
        EM_WRAP_ADL_FUNCTION(std, abs)
        EM_WRAP_ADL_FUNCTION(std, round)
        EM_WRAP_ADL_FUNCTION(std, floor)
        EM_WRAP_ADL_FUNCTION(std, ceil)
        EM_WRAP_ADL_FUNCTION(std, trunc)
        EM_WRAP_ADL_FUNCTION(std, modf)
        EM_WRAP_ADL_FUNCTION(std, nextafter)
    }

    // Absolute value.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( abs,, (const scalar auto &a) EM_RETURNS(detail::Funcs::abs_(a)) )
    // Round to a floating-point type.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( round,, (const floating_point_scalar auto &a) EM_RETURNS(detail::Funcs::round_(a)))

    // Round to an integral type.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR_EXT( iround,
        (template <integral_scalar I = int>), (EM_1<I>),, (const floating_point_scalar auto &a) EM_RETURNS(I(detail::Funcs::round_(a)))
    )

    // Round away from zero.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( round_maxabs,, (const floating_point_scalar auto &a) EM_RETURNS(a < 0 ? detail::Funcs::floor_(a) : detail::Funcs::ceil_(a)))
    // Round towards minus infinity.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( floor,, (const floating_point_scalar auto &a) EM_RETURNS(detail::Funcs::floor_(a)))
    // Round towards plus infinity.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( ceil,, (const floating_point_scalar auto &a) EM_RETURNS(detail::Funcs::ceil_(a)))

    // Remove the fractional part.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( trunc,, (const floating_point_scalar auto &a) EM_RETURNS(detail::Funcs::trunc_(a)))
    // Keep only the fractional part.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( frac,, (const floating_point_scalar auto &a) EM_RETURNS(detail::Funcs::modf_(a, 0)))
    // Return the fractional part, and write the integral part to the output parameter.
    // Maybe it would be nice to make `out_int` a pointer to match `std::modf()`, but currently our `apply_elementwise` doesn't understand pointers, and would need to be fixed.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( modf, (template <floating_point_scalar T>), (const T &a, T &out_int) EM_RETURNS(detail::Funcs::modf_(a, &out_int)))

    // `nextafter()`.
    // Here we require the same type for both operands. Trying to be too clever sounds pointless here.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( nextafter, (template <scalar T>), (const T &a, const T &b) EM_RETURNS(detail::Funcs::nextafter_(a, b)))




    inline namespace Common
    {
        using Math::make_floating_point;
        using Math::pi, Math::f_pi, Math::d_pi, Math::ld_pi;
        using Math::deg_to_rad;
        using Math::rad_to_deg;
        using Math::sign;
        using Math::diffsign;
        using Math::clamp_low;
        using Math::clamp_high;
        using Math::clamp;
        using Math::clamp_abs;
        using Math::clamp_var_low;
        using Math::clamp_var_high;
        using Math::clamp_var;
        using Math::clamp_var_abs;
        using Math::abs;
        using Math::round;
        using Math::iround;
        using Math::round_maxabs;
        using Math::floor;
        using Math::ceil;
        using Math::trunc;
        using Math::frac;
        using Math::modf;
        using Math::nextafter;
    }
}
