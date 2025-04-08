#pragma once

#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/returns.h"

#include <functional>

// This file defines functions to apply functions to vectors and vector-like types elementwise.

namespace em::Math
{
    // This a dummy ADL target, and a customization point for `apply_elementwise()` below.
    // When overriding this, it's important to reject the simplest `std::invoke(func, params...)` case,
    //   see `apply_elementwise_nontrivial()` below for explanation.
    // The customized versions of this must take a `<bool SameKind>` parameter. For example for vectors, if that's true,
    //   you must reject `vector + scalar` and only allow `vector + vector`.
    constexpr decltype(auto) _adl_em_apply_elementwise() {}
    // This version is given a functor that returns a bool (or a type that can be cast to one).
    // When the functor returns true, it returns that return value. Otherwise returns a default-constructed value of the same type.
    // Must always return by value.
    // The customized versions of this must take a `<bool SameKind>` parameter, same as `_adl_em_apply_elementwise()`.
    constexpr auto _adl_em_any_of_elementwise() {}

    // Takes N-ary functor and N arguments.
    // By default just calls it and returns the result.
    // If any argument is a vector, then any vector arguments must be vectors of the same size (or scalars),
    //   then calls the function N times and returns a vector of results.
    // Similarly supports combining other vector-like types with vectors and scalars.
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))

    // This version doesn't let you combine different entity kinds (vectors and non-vectors, etc).
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<true>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))

    // This version refuses to do `func(params...)`, and only accepts non-trivial cases (where e.g. vectors are involved).
    // This is useful to avoid circular `constraint satisfaction depends on itself`.
    // There's no `requires` that ensures this, instead we simply don't provide a second overload that calls `std::invoke`.
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise_same_kind_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_apply_elementwise<true>(EM_FWD(func), EM_FWD(params)...))


    // This version takes a functor that returns a bool (or a type that can be cast to one).
    // When the functor returns true, it returns that return value. Otherwise returns a default-constructed value of the same type.
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<true>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise_same_kind(auto &&func, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<false>(EM_FWD(func), EM_FWD(params)...))
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise_same_kind_nontrivial(auto &&func, auto &&... params) EM_RETURNS(_adl_em_any_of_elementwise<true>(EM_FWD(func), EM_FWD(params)...))
}
