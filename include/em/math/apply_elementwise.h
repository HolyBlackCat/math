#pragma once

#include "em/macros/meta/codegen.h"
#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/flag_enum.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/functors.h"
#include "em/macros/utils/returns.h"
#include "em/meta/casts.h"
#include "em/meta/functional.h"

// This file defines helpers to apply functions to vectors and vector-like types elementwise.
//
// The primary usages are:
//  1. Making a functor class:
//      1.1 Using `EM_SIMPLE_ELEMENTWISE[_SAME_KIND]_FUNCTOR()`, which is a modified version of the normal `EM_SIMPLE_FUNCTOR()`.
//      1.2 Wrapping your functor class with `em::Math::MakeElementwise[SameKind]`.
//  2. Calling `apply_elementwise()` or `any_of_elementwise()`.
//
// (1) doesn't expose the "any of" variant.
// (1.1) is recommended for library functions, or at least (1.2).
//
// I initially wanted to acheive (1.2) by creating macros that can be added to functors, but that doesn'y really work, because the macros (or wrappers)
//   first need to check that the function isn't applicable directly, and only then apply it elementwise.
// Checking this appears to be impossible with a macro. We need a wrapper functor that inherits from the user functor that it wraps.
//
// The user can still provide their own tag-invoke-style specializations for their types (if they really want to, for some reason),
//   by forward-declaring only the user functor (and not the wrapper), and then using it with `std::derived_from`.

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


    enum class ApplyElementwiseFlags
    {
        // Refuse to `std::invoke` the function directly (make it a SFINAE error).
        // Only allow something that involves vectors or some other types with elementwise behavior.
        nontrivial = 1 << 0,
        // Only allow combining vectors with vectors and not with scalars. Similarly for other special types.
        // Currently, when applying recursively (vectors of vectors), this forces the entire structure to match,
        //   e.g. prevents combining a vector of vectors with a vector of something else.
        // It seems to make more sense for my usecases, e.g. we use this in vector's `==` to disable `vector == scalar`,
        //   and it also makes sense to disable `vec<vec<int>> == vec<int>`.
        same_kind = 1 << 1,
    };
    EM_FLAG_ENUM(ApplyElementwiseFlags)


    EM_CODEGEN(
        (ApplyElementwiseFn, _adl_em_apply_elementwise)
        (AnyOfElementwiseFn, _adl_em_any_of_elementwise)
    ,,
        // Requiring `F` to be a reference for simplicity. If you decide to support non-references one day,
        //   `EM_FWD(...)` needs to be replaced with `EM_FWD_EX(...)`.

        template <typename F, ApplyElementwiseFlags Flags>
        class EM_1 : public F
        {
            static constexpr bool same_kind = bool(Flags & ApplyElementwiseFlags::same_kind);

          public:
            using F::operator();

            [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&... params)
            EM_RETURNS_REQ EM_P(
                // This condition ensures that we prefer calling the function as-is over elementwise.
                !requires{Meta::static_cast_to_cvref<F>(EM_FWD(self))(EM_FWD(params)...);},
                EM_2<same_kind> EM_P(EM_FWD(self), EM_FWD(params)...)
            )
        };

        template <typename F, ApplyElementwiseFlags Flags> requires(bool(Flags & ApplyElementwiseFlags::nontrivial))
        class EM_1<F, Flags> : public F
        {
            static constexpr bool same_kind = bool(Flags & ApplyElementwiseFlags::same_kind);

          public:
            // In this specialization we intentionally don't `using` the call operator of `F`.
            // Also here we don't need the `requires` condition on this one, because there's inherited one to disambiguate with.
            // We're here stripping the `nontrivial` flag when recursing, because otherwise this flag makes the function uncallable.
            [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&... params) EM_RETURNS EM_P(EM_2<same_kind> EM_P(EM_1<F, Flags & ~ApplyElementwiseFlags::nontrivial>{Meta::static_cast_to_cvref<F>(EM_FWD(self))}, EM_FWD(params)...))
        };
    )

    // Some helpers.
    template <typename F>
    using MakeElementwise = ApplyElementwiseFn<F, {}>;
    template <typename F>
    using MakeElementwiseSameKind = ApplyElementwiseFn<F, ApplyElementwiseFlags::same_kind>;

    // Applies the function either directly or elementwise to vectors or other similar types.
    // detail:  Note that we're using `Meta::ToFunctorObject` here, because `ApplyHelper` needs to inherit from whatever you give it.
    template <ApplyElementwiseFlags Flags = {}>
    [[nodiscard]] EM_TINY constexpr auto apply_elementwise(auto &&func, auto &&... params) EM_RETURNS(ApplyElementwiseFn<decltype(Meta::ToFunctorObject<Meta::ToFunctorFlags::ref>(EM_FWD(func))), Flags>{Meta::ToFunctorObject<Meta::ToFunctorFlags::ref>(EM_FWD(func))}(EM_FWD(params)...))

    // Applies the function either directly or elementwise to vectors or other similar types.
    template <ApplyElementwiseFlags Flags = {}>
    [[nodiscard]] EM_TINY constexpr auto any_of_elementwise(auto &&func, auto &&... params) EM_RETURNS(AnyOfElementwiseFn<decltype(Meta::ToFunctorObject<Meta::ToFunctorFlags::ref>(EM_FWD(func))), Flags>{Meta::ToFunctorObject<Meta::ToFunctorFlags::ref>(EM_FWD(func))}(EM_FWD(params)...))
}

// Like `EM_SIMPLE_FUNCTOR()`, but can also act elementwise.
#define EM_SIMPLE_ELEMENTWISE_FUNCTOR(name_, ...) EM_SIMPLE_ELEMENTWISE_FUNCTOR_EXT(name_, (), (EM_1), __VA_ARGS__)

// Like `EM_SIMPLE_FUNCTOR()`, but can also act elementwise (only on the same kind of objects).
#define EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR(name_, ...) EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR_EXT(name_, (), (EM_1), __VA_ARGS__)

// The extended version of `EM_SIMPLE_ELEMENTWISE_FUNCTOR()`, see `EM_SIMPLE_FUNCTOR_EXT()`. This is primarily for making templates.
#define EM_SIMPLE_ELEMENTWISE_FUNCTOR_EXT(name_, _template_head_, type_pattern_, ...) \
    EM_SIMPLE_FUNCTOR_EXT(name_, _template_head_, (::em::Math::MakeElementwise<EM_UNWRAP_CODE(type_pattern_)>), __VA_ARGS__)

// The extended version of `EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR()`, see `EM_SIMPLE_FUNCTOR_EXT()`. This is primarily for making templates.
#define EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR_EXT(name_, _template_head_, type_pattern_, ...) \
    EM_SIMPLE_FUNCTOR_EXT(name_, _template_head_, (::em::Math::MakeElementwiseSameKind<EM_UNWRAP_CODE(type_pattern_)>), __VA_ARGS__)
