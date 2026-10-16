#pragma once

#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/returns.h"
#include "em/math/apply_elementwise.h"
#include "em/math/larger_type.h"
#include "em/math/namespaces.h"
#include "em/math/scalar.h"
#include "em/math/vector_traits.h"
#include "em/meta/casts.h"

#include <cmath>
#include <limits>
#include <utility>

// Some funcitons defined here are then added to `inline namespace Common` at the bottom, and some don't. They have to be interleaved, because they can call each other.
// Functions NOT in `Common` are marked with `// [not in Common]`.

// For now we don't include `em/math/vector.h` here, and only include `em/math/vector_traits.h`. This in theory allows accepting third-party vector types.
// Avoid using members of `vec` to support third-party vectors. E.g. use `vec_min(v)` instead of `v.min()`.

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
        EM_OVERLOAD // Clamp to a fixed range `0..`.
        (template <scalar T>),
        (const T &target) EM_RETURNS(target >= T{} ? target : T{})
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_high,
        (template <scalar T, scalar A, typename L = larger_t<T, A>>),
        (const T &target, const A &high) EM_RETURNS(target <= high ? L(target) : L(high))
        EM_OVERLOAD // Clamp to a fixed range `..1`.
        (template <scalar T>),
        (const T &target) EM_RETURNS(target <= T(1) ? target : T(1))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp,
        (template <scalar T, scalar A, scalar B, typename L = larger_t<T, A, B>>),
        (const T &target, const A &low, const B &high) EM_RETURNS(target >= low ? (target <= high ? L(target) : L(high)) : L(low))
        EM_OVERLOAD // Clamp to a fixed range `0..1`.
        (template <scalar T>),
        (const T &target) EM_RETURNS(target >= T{} ? (target <= T(1) ? target : T(1)) : T{})
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_abs,
        ,(const auto &target, const auto &abs_limit) EM_RETURNS(clamp(target, -abs_limit, abs_limit))
        EM_OVERLOAD // Clamp to a fixed range `-1..1`.
        (template <scalar T>),
        (const T &target) EM_RETURNS(clamp(target, T(-1), T(1)))
    )

    // Takes a variable by reference and clamps it from one or both sides.
    // Note that NaN gets replaced with the bound, so we need to be careful about how we compare things.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_low,
        (template <scalar T, scalar A> requires can_safely_convert_to<A, T>),
        (T &target, const A &low) EM_RETURNS(target >= low ? void() : void(target = low))
        EM_OVERLOAD // Clamp to a fixed range `0..`.
        (template <scalar T>),
        (T &target) EM_RETURNS(target >= T{} ? void() : void(target = T{}))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_high,
        (template <scalar T, scalar A> requires can_safely_convert_to<A, T>),
        (T &target, const A &high) EM_RETURNS(target <= high ? void() : void(target = high))
        EM_OVERLOAD // Clamp to a fixed range `1..`.
        (template <scalar T>),
        (T &target) EM_RETURNS(target <= T(1) ? void() : void(target = T(1)))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var,
        // This isn't implemented in terms of `clamp_var_{high,low}` to better match how `clamp()` behaves with inverted bounds and NaN.
        (template <scalar T, scalar A, scalar B> requires can_safely_convert_to<A, T> && can_safely_convert_to<B, T>),
        (T &target, const A &low, const B &high) EM_RETURNS(target >= low ? (target <= high ? void() : void(target = high)) : void(target = low))
        EM_OVERLOAD // Clamp to a fixed range `0..1`.
        (template <scalar T>),
        (T &target) EM_RETURNS(target >= T{} ? (target <= T(1) ? void() : void(target = T(1))) : void(target = T{}))
    )
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( clamp_var_abs,,
        (scalar auto &target, const scalar auto &abs_limit) EM_RETURNS(clamp_var(target, -abs_limit, abs_limit))
        EM_OVERLOAD // Clamp to a fixed range `-1..1`.
        (template <scalar T>),
        (T &target) EM_RETURNS(clamp_var(target, T(-1), T(1)))
    )


    namespace detail::Funcs
    {
        // EM_WRAP_ADL_FUNCTION(std, abs) // Not constexpr yet! Need to reimplement it myself:
        constexpr auto abs(const scalar auto &s) EM_RETURNS(s >= 0 ? s : -s)
        EM_WRAP_ADL_FUNCTION(Funcs, abs)

        EM_WRAP_ADL_FUNCTION(std, round)
        EM_WRAP_ADL_FUNCTION(std, floor)
        EM_WRAP_ADL_FUNCTION(std, ceil)
        EM_WRAP_ADL_FUNCTION(std, trunc)
        EM_WRAP_ADL_FUNCTION(std, modf)
        EM_WRAP_ADL_FUNCTION(std, nextafter)
        EM_WRAP_ADL_FUNCTION(std, pow)
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
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( nextafter, (template <floating_point_scalar T>), (const T &a, const T &b) EM_RETURNS(detail::Funcs::nextafter_(a, b)))

    // Floating-point power.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( pow, (template <scalar T>), (const T &a, const T &b) EM_RETURNS(detail::Funcs::pow_(a, b)))


    // Integer division, modified for negative values of `a` to be periodic:
    //           i : -4  -3  -2  -1  0  1  2  3  4
    // div_ex(i,2) : -2  -2  -1  -1  0  0  1  1  2
    // If `b` is negative, flips the sign of the result, just like regular division.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( div_ex,, (const integral_scalar auto &a, const integral_scalar auto &b) EM_RETURNS(a >= 0 ? a / b : (a + 1) / b - sign(b)) )

    // Integer division, modified for negative values of `a` to be periodic:
    //           i : -4  -3  -2  -1  0  1  2  3  4
    // div_ex(i,3) :  2   0   1   2  0  1  2  0  1
    // The sign of `b` is ignored, just like what `%` does.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( mod_ex,, (const integral_scalar auto &a, const integral_scalar auto &b) EM_RETURNS(a >= 0 ? a % b : abs(b) - 1 + (a + 1) % b) )


    // Performs division, rounding away from zero. Handles both integers and fractional numbers.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( div_maxabs,
        ,(const integral_scalar auto &a, const integral_scalar auto &b) EM_RETURNS((a + (abs(b) - 1) * sign(a)) / b)
        EM_OVERLOAD
        (template <scalar A, scalar B, typename L = larger_t<A, B>>),
        (const A &a, const B &b) EM_RETURNS(round_maxabs(L(a) / L(b)))
    )


    // Computes `a ^ b`, where `b` is a non-negative integer.
    // Negative `b` is treated as zero.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( ipow,,
        // This implementation could handle vector `a` transparently, but it's easier to use `ELEMENTWISE_FUNCTOR` to accept other types that may not have the right overloaded operators.
        (scalar auto a, integral_scalar auto b)
        {
            decltype(a) ret = 1;
            while (b > 0)
            {
                if (b & 1)
                    ret *= a;
                a *= a;
                b >>= 1;
            }
            return ret;
        }
    )


    // Computes the smooth step function. Doesn't clamp `x`.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( smoothstep,,
        // This implementation could handle vectors transparently, but it's easier to use `ELEMENTWISE_FUNCTOR` to accept other types that may not have the right overloaded operators.
        (const floating_point_scalar auto &x) EM_RETURNS( (3 - 2*x) * x*x )
    )


    // Computes the smooth step function. Doesn't clamp `x`.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( mix,
        // This implementation could handle vectors transparently, but it's easier to use `ELEMENTWISE_FUNCTOR` to accept other types that may not have the right overloaded operators.
        (template <scalar F, scalar A, scalar B, typename L = larger_t<A, B>>),
        (const F &factor, const A &a, const B &b) EM_RETURNS( L(a) * (F(1)-factor) + L(b) * factor )
    )

    // Swaps the elements of `a` and `b` such that every element of `a` is less or equal than the respective element of `b`.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( sort_two_var,
        (template <typename T>), // This doesn't even require scalars. So for this to function correctly, vector comparisons must not return bool.
        (T &a, T &b) EM_RETURNS( b < a ? std::swap(a, b) : void() )
    )

    // Returns `std::pair<T, T>` such that the first element is smaller or equal to the second.
    // Note, this is not elementwise, since it must return a single `std::pair` and not a vector of those.
    EM_SIMPLE_FUNCTOR( sort_two,
        (template <typename T>),
        (T a, T b) EM_RETURNS( sort_two_var(a, b), std::pair<T, T>(std::move(a), std::move(b)) )
        EM_OVERLOAD // Handle different parameter types. Turns out overloading like this is fine.
        (template <typename A, typename B, typename L = larger_t<A, B>>),
        (A &&a, B &&b) EM_RETURNS( Fn_sort_two::operator()(Meta::static_cast_or_return_ref<L>(EM_FWD(a)), Meta::static_cast_or_return_ref<L>(EM_FWD(b))) )
    )


    // Min/max:

    namespace detail::MinMax
    {
        template <bool IsMax>
        struct FnMinMaxLow
        {
            // One argument -> return as is by value.
            // This verison accepts types that are not comparable too. Primarily to support vectors. We could check for single-argument `apply_elementwise()`, but who cares.
            // Maybe it is actually better to continue allowing this to support types that don't have `apply_elementwise()`.
            [[nodiscard]] EM_TINY constexpr auto operator()(auto &&a) const EM_RETURNS(auto(EM_FWD(a)))
            // Two arguments of the same type ignoring cvref -> pick the smallest one.
            [[nodiscard]] EM_TINY constexpr auto operator()(auto &&a, auto &&b) const EM_RETURNS(auto((a < b ? !IsMax : IsMax) ? EM_FWD(a) : EM_FWD(b)))
            // Three+ arguments -> fold.
            // Recursing this way needs `(*this)(...)`. Static call operators don't work, and free functions can't recurse into themselves using `EM_RETURNS(...)`.
            [[nodiscard]] EM_TINY constexpr auto operator()(auto &&a, auto &&b, auto &&c, auto &&... d) const EM_RETURNS((*this)(EM_FWD(a), (*this)(EM_FWD(b), EM_FWD(c), EM_FWD(d)...)))
        };
    }

    // A functor class implementing `min()` and `max()`. We declare those as variables below.
    template <bool IsMax>
    struct FnMinMax
    {
        // Convert to the common `larger_t` if needed and forward to the underlying implementation.
        template <typename ...P>
        [[nodiscard]] EM_TINY constexpr auto operator()(P &&... params) const EM_RETURNS( detail::MinMax::FnMinMaxLow<IsMax>{}(Meta::static_cast_or_return_ref<larger_t<std::remove_cvref_t<P>...>>(EM_FWD(params))...) )
    };

    inline constexpr ApplyElementwiseFn<FnMinMax<false>, {}> min;
    inline constexpr ApplyElementwiseFn<FnMinMax<true>, {}> max;


    // [not in Common]
    // Like `nextafter()`, but works with integers as well.
    // `next_value_towards(a, b)` returns the next representable value after `a` in the direction towards `b`.
    // `b` must be convertible to `a` without narrowing. The return type matches that of `a`.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( next_value_towards,
        (template <floating_point_scalar A, can_safely_convert_to<A> B>),
        (const A &value, const B &target) EM_RETURNS( nextafter(value, A(target)) )
        EM_OVERLOAD
        (template <integral_scalar A, can_safely_convert_to<A> B>),
        (const A &value, const B &target) EM_RETURNS( value + diffsign(A(target), value) ) // The plain `sign()` could overflow here.
    )

    // [not in Common]
    // Returns the next representable value.
    // Refuses to increment the largest representable value, and returns it unchanged.
    // If asked to increment infinity in either direction, returns the closest representable value.
    // If given NaN, returns NaN.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( next_value,
        (template <scalar T>),
        (const T &value) EM_RETURNS( next_value_towards(value, std::numeric_limits<T>::max()) )
    )
    // [not in Common]
    // Same, but returns the previous representable value.
    EM_SIMPLE_ELEMENTWISE_FUNCTOR( prev_value,
        (template <scalar T>),
        (const T &value) EM_RETURNS( next_value_towards(value, std::numeric_limits<T>::lowest()) )
    )

    // [not in Common]
    // Return the min/max element of a vector.
    // This is primarily for third-party vectors. Prefer `.min()`/`.max()` for our own vectors.
    // By dropping the `vector_cvref` concept we could accept any type, and then return it unchanged.
    EM_SIMPLE_FUNCTOR(
        vec_min,,
        (vector_cvref auto &&value) EM_RETURNS( vec_apply(value, min) )
    )
    EM_SIMPLE_FUNCTOR(
        vec_max,,
        (vector_cvref auto &&value) EM_RETURNS( vec_apply(value, max) )
    )


    // Shrinks a vector as little as possible to give it specific proportions.
    // Always returns a floating-point type.
    EM_SIMPLE_FUNCTOR(
        shrink_to_proportions,
        (template <typename A, typename B, typename L = floating_point_t<A, B>>),
        (const A &value, const B &proportions) EM_RETURNS( vec_min(L(value) / L(proportions)) * L(proportions) )
    )
    // Expands a vector as little as possible to give it specific proportions.
    // Always returns a floating-point type.
    EM_SIMPLE_FUNCTOR(
        expand_to_proportions,
        (template <typename A, typename B, typename L = floating_point_t<A, B>>),
        (const A &value, const B &proportions) EM_RETURNS( vec_max(L(value) / L(proportions)) * L(proportions) )
    )


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
        using Math::pow;
        using Math::div_ex;
        using Math::mod_ex;
        using Math::div_maxabs;
        using Math::ipow;
        using Math::smoothstep;
        using Math::mix;
        using Math::sort_two;
        using Math::sort_two_var;
        using Math::min;
        using Math::max;
        using Math::shrink_to_proportions;
        using Math::expand_to_proportions;
    }
}
