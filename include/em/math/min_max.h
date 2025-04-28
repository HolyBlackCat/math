#pragma once

#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/returns.h"
#include "em/macros/utils/sfinae.h"
#include "em/math/apply_elementwise.h"
#include "em/math/larger_type.h"
#include "em/math/namespaces.h"
#include "em/meta/cvref.h"
#include "em/meta/deduce.h"

#include <concepts>

namespace em::Math
{
    namespace detail::MinMax
    {
        // Check that `<`, `>` exist and are sane, for use with our `min()`, `max()`.
        // There's a standard concept `std::totally_ordered` that does something similar, but it has semantic requirements
        //   that floating-point numbers violate, so we don't use it here.
        template <typename T>
        concept Comparable = requires(const T &a, const T &b)
        {
            {a < b} -> std::same_as<bool>;
        };

        // If the input is same as `T` ignoring cvref, returns it by reference with perfect forwarding.
        // If it's a different type, casts it to `T` and returns that by value.
        template <Meta::cvref_unqualified T, Meta::Deduce..., Meta::same_ignoring_cvref<T> U>
        [[nodiscard]] EM_TINY auto CastOrForward(U &&value) EM_RETURNS(EM_FWD(value))
        template <Meta::cvref_unqualified T, Meta::Deduce..., typename U>
        [[nodiscard]] EM_TINY auto CastOrForward(U &&value) EM_RETURNS(T(EM_FWD(value)))
    }

    // A functor class implementing `min()` and `max()`. We declare those as variables below.
    template <bool IsMax>
    struct FnMinMax
    {
        EM_APPLICABLE_ELEMENTWISE

        // One argument -> return as is by value.
        // This verison accepts types that are not comparable too. Primarily to support vectors. We could check for single-argument `apply_elementwise()`, but who cares.
        // Maybe it is actually better to continue allowing this to support types that don't have `apply_elementwise()`.
        template <typename A>
        [[nodiscard]] EM_TINY constexpr auto operator()(A &&a) const EM_RETURNS(auto(EM_FWD(a)))
        // Two arguments of the same type ignoring cvref -> pick the smallest one.
        template <detail::MinMax::Comparable A, detail::MinMax::Comparable B> requires Meta::same_ignoring_cvref<A, B>
        [[nodiscard]] EM_TINY constexpr auto operator()(A &&a, B &&b) const EM_RETURNS(auto((a < b ? !IsMax : IsMax) ? EM_FWD(a) : EM_FWD(b)))
        // Two arguments of different types -> convert to the common larger type and pass to the previous overload.
        // Note that if one of the two arguments matches the larger type, `CastOrForward()` doesn't copy/move it, and instead returns by reference.
        // Note that we need a negative old-style SFINAE here. This having lower precedence than the overload with `requires` above
        //   doens't seem to be enough, in that case Clang fails with an infinite instantiation recursion.
        template <detail::MinMax::Comparable A, detail::MinMax::Comparable B, EM_CHECK(!Meta::same_ignoring_cvref<A, B>)>
        [[nodiscard]] EM_TINY constexpr auto operator()(A &&a, B &&b) const EM_RETURNS((*this)(detail::MinMax::CastOrForward<larger_t<A, B>>(EM_FWD(a)), detail::MinMax::CastOrForward<larger_t<A, B>>(EM_FWD(b))))
        // Three+ arguments -> fold. Must use `Meta::Fold` because `EM_RETURNS()` can't recursively call the same function,
        //   because you can't `decltype(...)` it for the return type, and `noexcept(...)` doesn't work too.
        template <detail::MinMax::Comparable A, detail::MinMax::Comparable B, detail::MinMax::Comparable C, detail::MinMax::Comparable ...D>
        [[nodiscard]] EM_TINY constexpr auto operator()(A &&a, B &&b, C &&c, D &&... d) const EM_RETURNS((*this)(EM_FWD(a), (*this)(EM_FWD(b), EM_FWD(c), EM_FWD(d)...)))
    };
    inline constexpr FnMinMax<false> min;
    inline constexpr FnMinMax<true> max;

    inline namespace Common
    {
        using Math::min;
        using Math::max;
    }
}
