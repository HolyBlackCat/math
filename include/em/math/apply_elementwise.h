#pragma once

#include "em/macros/portable/always_inline.h"
#include "em/macros/portable/artificial.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/returns.h"
#include "em/macros/utils/weak_bool_cast.h"

#include <functional>
#include <utility>

// This file defines functions to apply functions to vectors and vector-like types elementwise.

namespace em::Math
{
    // This a dummy ADL target, and a customization point for `apply_elementwise()` below.
    // When overriding this, it's important to reject the simplest `std::invoke(func, params...)` case,
    //   see `apply_elementwise_nontrivial()` below for explanation.
    constexpr auto _adl_em_apply_elementwise() {}
    // This version is given a functor that returns a bool (or a type that can be cast to one).
    // When the functor returns true, it returns that return value. Otherwise returns a default-constructed value of the same type.
    constexpr auto _adl_em_any_of_elementwise() {}

    // Takes N-ary functor and N arguments.
    // By default just calls it and returns the result.
    // If any argument is a vector, then any vector arguments must be vectors of the same size (or scalars),
    //   then calls the function N times and returns a vector of results.
    // Similarly supports combining other vector-like types with vectors and scalars.
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto apply_elementwise(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto apply_elementwise(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))

    // This version doesn't let you combine different entity kinds (vectors and non-vectors, etc).
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto apply_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<true>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto apply_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))

    // This version refuses to do `func(params...)`, and only accepts non-trivial cases (where e.g. vectors are involved).
    // This is useful to avoid circular `constraint satisfaction depends on itself`.
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto apply_elementwise_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto apply_elementwise_same_kind_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<true>(EM_FWD(func), EM_FWD(params)...))


    // This version takes a functor that returns a bool (or a type that can be cast to one).
    // When the functor returns true, it returns that return value. Otherwise returns a default-constructed value of the same type.
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto any_of_elementwise(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto any_of_elementwise(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto any_of_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<true>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto any_of_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto any_of_elementwise_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto any_of_elementwise_same_kind_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<true>(EM_FWD(func), EM_FWD(params)...))
}
