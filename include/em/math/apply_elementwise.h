#pragma once

#include "em/macros/meta/codegen.h"
#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/flag_enum.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/returns.h"
#include "em/meta/concepts.h"

#include <functional>

// This file defines helpers to apply functions to vectors and vector-like types elementwise.
//
// The two primary usages are:
//   1. Making a functor class and marking it with a `EM_APPLICABLE_ELEMENTWISE[_SAME_KIND]` macro.
//   2. Calling `apply_elementwise()` or `any_of_elementwise()`.
//
// The macros don't expose the "any of" variant.
// The macros are recommended for library functions.

// Add this to a functor class to automatically make it work elementwise.
#define EM_APPLICABLE_ELEMENTWISE \
    EM_APPLICABLE_ELEMENTWISE_MAYBE_SAME_KIND(false)

// Add this to a functor class to automatically make it work elementwise, but don't allow mixing vectors and non-vectors.
#define EM_APPLICABLE_ELEMENTWISE_SAME_KIND \
    EM_APPLICABLE_ELEMENTWISE_MAYBE_SAME_KIND(true)

// A generic version, either `EM_APPLICABLE_ELEMENTWISE` or `EM_APPLICABLE_ELEMENTWISE_SAME_KIND` depending on a bool.
// This bool can be a template parameter.
#define EM_APPLICABLE_ELEMENTWISE_MAYBE_SAME_KIND(.../*same_kind*/) \
    [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&a, auto &&... b) \
    EM_RETURNS(_adl_em_apply_elementwise<__VA_ARGS__>(EM_FWD(self), EM_FWD(a), EM_FWD(b)...))

namespace em::Math
{
    enum class ApplyElementwiseFlags
    {
        // Refuse to `std::invoke` the function directly (make it a SFINAE error).
        // Only allow something that involves vectors or some other types with elementwise behavior.
        nontrivial = 1 << 0,
        // Only allow combining vectors with vectors and not with scalars. Similarly for other special types.
        // Currently, when applying recursively (vectors of vectors), this forces the entire structure to match,
        //   e.g. prevents combining a vector of vectors with a vector of something else.
        // Firstly because this is easier to implement, and secondly because it makes more sense for my usecases,
        //   e.g. we use this in vector's `==` to disable `vector == scalar`, and it also makes sense to disable `vec<vec<int>> == vec<int>`.
        same_kind = 1 << 1,
    };
    EM_FLAG_ENUM(ApplyElementwiseFlags)

    namespace detail::ApplyElementwise
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

        // We need this functor to recruse in `apply_elementwise()` and `any_of_elementwise()`,
        //   because they can't call themselves in their `EM_RETURNS(...)`.
        //
        EM_CODEGEN(
            (ApplyHelper, _adl_em_apply_elementwise)
            (AnyOfHelper, _adl_em_any_of_elementwise)
        ,,
            // Requiring `F` to be a reference for simplicity. If you decide to support non-references one day,
            //   `EM_FWD(...)` needs to be replaced with `EM_FWD_EX(...)`.

            // There are two functors here. `EM_1` is the primary one.
            // `EM_1 _Simple` is the implementation detail of `EM_1`. It is necessary because when recrusing here,
            //   we need to adjust some flags (remove `nontrivial`, otherwise it doesn't work at all),
            //   and the naive approach (constructing the same class with different flags) doesn't work, because apparently
            //   it isn't complete yet in `EM_RETURNS(...)`.
            // This forces us to make a second class.

            template <Meta::reference F, bool SameKind>
            struct EM_CAT EM_P(EM_1, _Simple)
            {
                F func;

                // Note: Here and below `EM_FWD(self.func)` is intentional, as opposed to `EM_FWD(self).func`, because the latter doesn't respect
                //   `func` being an rvalue reference, and we don't really care about `self` being rvalue or not.

                [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&... params) EM_RETURNS(std::invoke(EM_FWD(self.func), EM_FWD(params)...))
                [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&... params) EM_RETURNS EM_P(EM_2<SameKind>(EM_FWD(self), EM_FWD(params)...))
            };

            template <Meta::reference F, ApplyElementwiseFlags Flags>
            struct EM_1
            {
                static constexpr bool same_kind = bool(Flags & ApplyElementwiseFlags::same_kind);
                static constexpr bool nontrivial = bool(Flags & ApplyElementwiseFlags::nontrivial);

              public:
                F func;

                [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&... params) EM_RETURNS_REQ(!nontrivial, std::invoke(EM_FWD(self.func), EM_FWD(params)...))
                [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&... params) EM_RETURNS EM_P(EM_CAT EM_P(EM_1, _Simple)<F, same_kind>{EM_FWD(self.func)}(EM_FWD(params)...))
            };
        )
    }

    // Applies the function either directly or elementwise to vectors or other similar types.
    template <ApplyElementwiseFlags Flags = {}>
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise(auto &&func, auto &&... params) EM_RETURNS(detail::ApplyElementwise::ApplyHelper<decltype(func), Flags>{EM_FWD(func)}(EM_FWD(params)...))

    // Applies the function either directly or elementwise to vectors or other similar types.
    template <ApplyElementwiseFlags Flags = {}>
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise(auto &&func, auto &&... params) EM_RETURNS(detail::ApplyElementwise::AnyOfHelper<decltype(func), Flags>{EM_FWD(func)}(EM_FWD(params)...))
}
