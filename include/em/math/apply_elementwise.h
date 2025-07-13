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
//      1.2 Wrapping your functor class with `em::Math::{Apply,AllOf,AnyOf>ElementwiseFn`.
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
    // This should call `std::invoke(func, get_element_somehow(params)...)` one or more times.
    // When overriding this, it's important to reject the simplest `std::invoke(func, params...)` case,
    //   see `apply_elementwise_nontrivial()` below for explanation.
    // The customized versions of this must take a `<bool SameKind>` parameter. For example for vectors, if that's true,
    //   you must reject `vector + scalar` and only allow `vector + vector`.
    // Note that the customized versions must support the user callback returning void, and in that case probably they should return void too.
    constexpr decltype(auto) _adl_em_apply_elementwise(/*func, params...*/) {}
    // This version is given a functor that returns a bool (or a type that can be cast to one).
    // When the functor returns true, it returns that return value. Otherwise returns a default-constructed value of the same type.
    // Must always return by value.
    // The customized versions of this must take a `<bool SameKind>` parameter, same as `_adl_em_apply_elementwise()`.
    constexpr auto _adl_em_any_of_elementwise(/*func, params...*/) {}


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

    // "Apply elementwise" and "any of elementwise".
    EM_CODEGEN(
        (ApplyElementwiseFn,  _adl_em_apply_elementwise , apply_elementwise ,                         )
        (AnyOfElementwiseFn,  _adl_em_any_of_elementwise, any_of_elementwise,                         )
        (AllOfElementwiseFn, !_adl_em_any_of_elementwise, all_of_elementwise, Meta::MakeNegatedFuncRef)
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
                EM_2<same_kind> EM_P(
                    EM_4(EM_FWD(self)),
                    EM_FWD(params)...
                )
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
            [[nodiscard]] EM_TINY constexpr auto operator()(this auto &&self, auto &&... params)
            EM_RETURNS EM_P(
                EM_2<same_kind> EM_P(
                    EM_4 EM_P(EM_1<Meta::FuncRef<Meta::copy_cvref<decltype(self), F>>, Flags & ~ApplyElementwiseFlags::nontrivial>{Meta::FuncRef(Meta::static_cast_to_cvref<F>(EM_FWD(self)))}),
                    EM_FWD(params)...
                )
            )
        };

        // The user-facing function: `apply_elementwise`, `any_of_elementwise`, etc.
        // Applying `FuncRef()` here to force the functor to have a class type, so we can inherit from it. And to avoid copying, yes.
        template <ApplyElementwiseFlags Flags = {}>
        [[nodiscard]] EM_TINY constexpr auto EM_3(auto &&func, auto &&... params) EM_RETURNS EM_P(EM_1<decltype(Meta::FuncRef(EM_FWD(func))), Flags>{Meta::FuncRef(EM_FWD(func))}(EM_FWD(params)...))
    )
}

// Like `EM_SIMPLE_FUNCTOR()`, but can also act elementwise.
#define EM_SIMPLE_ELEMENTWISE_FUNCTOR(name_, deduced_targs_and_extras_, ...) EM_SIMPLE_ELEMENTWISE_FUNCTOR_EXT(name_, (), (EM_1), deduced_targs_and_extras_, __VA_ARGS__)

// Like `EM_SIMPLE_FUNCTOR()`, but can also act elementwise (only on the same kind of objects).
#define EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR(name_, deduced_targs_and_extras_, ...) EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR_EXT(name_, (), (EM_1), deduced_targs_and_extras_, __VA_ARGS__)

// The extended version of `EM_SIMPLE_ELEMENTWISE_FUNCTOR()`, see `EM_SIMPLE_FUNCTOR_EXT()`. This is primarily for making templates.
#define EM_SIMPLE_ELEMENTWISE_FUNCTOR_EXT(name_, template_head_, type_pattern_, deduced_targs_and_extras_, ...) DETAIL_EM_SIMPLE_ELEMENTWISE_FUNCTOR(ApplyElementwiseFn, {}, name_, template_head_, type_pattern_, deduced_targs_and_extras_, __VA_ARGS__)

// The extended version of `EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR()`, see `EM_SIMPLE_FUNCTOR_EXT()`. This is primarily for making templates.
#define EM_SIMPLE_ELEMENTWISE_SAME_KIND_FUNCTOR_EXT(name_, template_head_, type_pattern_, deduced_targs_and_extras_, ...) DETAIL_EM_SIMPLE_ELEMENTWISE_FUNCTOR(ApplyElementwiseFn, ::same_kind, name_, template_head_, type_pattern_, deduced_targs_and_extras_, __VA_ARGS__)

#define EM_SIMPLE_ANY_OF_FUNCTOR(name_, deduced_targs_and_extras_, ...)           EM_SIMPLE_ANY_OF_FUNCTOR_EXT          (name_, (), (EM_1), deduced_targs_and_extras_, __VA_ARGS__)
#define EM_SIMPLE_ANY_OF_SAME_KIND_FUNCTOR(name_, deduced_targs_and_extras_, ...) EM_SIMPLE_ANY_OF_SAME_KIND_FUNCTOR_EXT(name_, (), (EM_1), deduced_targs_and_extras_, __VA_ARGS__)
#define EM_SIMPLE_ANY_OF_FUNCTOR_EXT(name_, template_head_, type_pattern_, deduced_targs_and_extras_, ...)           DETAIL_EM_SIMPLE_ELEMENTWISE_FUNCTOR(AnyOfElementwiseFn, {}, name_, template_head_, type_pattern_, deduced_targs_and_extras_, __VA_ARGS__)
#define EM_SIMPLE_ANY_OF_SAME_KIND_FUNCTOR_EXT(name_, template_head_, type_pattern_, deduced_targs_and_extras_, ...) DETAIL_EM_SIMPLE_ELEMENTWISE_FUNCTOR(AnyOfElementwiseFn, ::same_kind, name_, template_head_, type_pattern_, deduced_targs_and_extras_, __VA_ARGS__)

#define EM_SIMPLE_ALL_OF_FUNCTOR(name_, deduced_targs_and_extras_, ...)           EM_SIMPLE_ALL_OF_FUNCTOR_EXT          (name_, (), (EM_1), deduced_targs_and_extras_, __VA_ARGS__)
#define EM_SIMPLE_ALL_OF_SAME_KIND_FUNCTOR(name_, deduced_targs_and_extras_, ...) EM_SIMPLE_ALL_OF_SAME_KIND_FUNCTOR_EXT(name_, (), (EM_1), deduced_targs_and_extras_, __VA_ARGS__)
#define EM_SIMPLE_ALL_OF_FUNCTOR_EXT(name_, template_head_, type_pattern_, deduced_targs_and_extras_, ...)           DETAIL_EM_SIMPLE_ELEMENTWISE_FUNCTOR(AllOfElementwiseFn, {}, name_, template_head_, type_pattern_, deduced_targs_and_extras_, __VA_ARGS__)
#define EM_SIMPLE_ALL_OF_SAME_KIND_FUNCTOR_EXT(name_, template_head_, type_pattern_, deduced_targs_and_extras_, ...) DETAIL_EM_SIMPLE_ELEMENTWISE_FUNCTOR(AllOfElementwiseFn, ::same_kind, name_, template_head_, type_pattern_, deduced_targs_and_extras_, __VA_ARGS__)


// This is used internally to implement the macros above.
#define DETAIL_EM_SIMPLE_ELEMENTWISE_FUNCTOR(functor_, flag_, name_, template_head_, type_pattern_, deduced_targs_and_extras_, ...) \
    EM_SIMPLE_FUNCTOR_EXT(name_, template_head_, (::em::Math::functor_<EM_UNWRAP_CODE(type_pattern_), ::em::Math::ApplyElementwiseFlags flag_>), deduced_targs_and_extras_, __VA_ARGS__)
