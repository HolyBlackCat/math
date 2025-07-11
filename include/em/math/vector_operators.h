#pragma once

#include "em/macros/meta/common.h"
#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/returns.h"
#include "em/math/apply_elementwise.h"
#include "em/math/operator_functors.h"
#include "em/meta/common.h"

namespace em::Math::VectorOps
{
    // This is a CRTP base, giving a class vector operators (via ADL).
    // The operators are implemented using `apply_elementwise`.
    template <typename Derived>
    struct EnableVectorOps
    {
        // If you provide this function, you can optionally disable some operators even if `apply_elementwise()` is callable for them.
        // `operator_functor` is either one of `em::Math::Ops::...`, or
        // `A` and `B` are always references here, lvalue or rvalue ones.
        // auto _adl_em_vec_allow_operator(operator_functor, tag<A> [,tag<B>])
    };

    // This is used to implement `==`.
    struct NotEqual
    {
        template <typename T, typename U>
        [[nodiscard]] EM_TINY static constexpr auto operator()(T &&t, U &&u) EM_RETURNS(EM_FWD(t) != EM_FWD(u) ? true : false)
    };

    // This is used to implement `<=>`.
    struct CompareThreeWay
    {
        template <typename T>
        struct Result
        {
            T value = T::equivalent;
            [[nodiscard]] explicit constexpr operator bool() const noexcept {return value != 0;}
        };

        template <typename T, typename U>
        [[nodiscard]] EM_TINY static constexpr auto operator()(T &&t, U &&u) EM_RETURNS(Result(EM_FWD(t) <=> EM_FWD(u)))
    };

    // Allow everything by default, except for pure scalars.
    template <typename ...P>
    constexpr bool _adl_em_vec_allow_operator(auto &&, Meta::Tag<P>...) {return true;}

    #define DETAIL_EM_VEC_UNARY_OP(name_, op_, ...) \
        /* Unary. */\
        template <typename A> requires(_adl_em_vec_allow_operator(Ops::name_{}, Meta::Tag<A &&>{})) \
        [[nodiscard]] EM_TINY constexpr auto operator op_(A &&a) EM_RETURNS((apply_elementwise<ApplyElementwiseFlags::nontrivial>)(Ops::name_{}, EM_FWD(a)))
    EM_MATH_OPS_UNARY(DETAIL_EM_VEC_UNARY_OP)
    #undef DETAIL_EM_VEC_UNARY_OP

    #define DETAIL_EM_VEC_BINARY_OP(name_, op_, ...) \
        /* Binary. */\
        template <typename A, typename B> requires(_adl_em_vec_allow_operator(Ops::name_{}, Meta::Tag<A &&>{}, Meta::Tag<B &&>{})) \
        [[nodiscard]] EM_TINY constexpr auto operator op_(A &&a, B &&b) EM_RETURNS((apply_elementwise<ApplyElementwiseFlags::nontrivial>)(Ops::name_{}, EM_FWD(a), EM_FWD(b))) \
        /* Assignment. */\
        template <typename A, typename B> requires(_adl_em_vec_allow_operator(Ops::name_{}, Meta::Tag<A &&>{}, Meta::Tag<B &&>{})) \
        EM_TINY constexpr auto operator EM_CAT(op_,=)(A &&a, B &&b) EM_RETURNS((void((apply_elementwise<ApplyElementwiseFlags::nontrivial>)(Ops::EM_CAT(name_, Assign){}, EM_FWD(a), EM_FWD(b))), a))
    EM_MATH_OPS_BINARY(DETAIL_EM_VEC_BINARY_OP)
    #undef DETAIL_EM_VEC_BINARY_OP

    // Unlike other operators, this uses `...elementwise_same_kind...()` to reject some weird combinations.
    template <typename A, typename B> requires(_adl_em_vec_allow_operator(NotEqual{},        Meta::Tag<const A &>(), Meta::Tag<const B &>()))
    [[nodiscard]] EM_TINY constexpr auto operator==(const A &a, const B &b) EM_RETURNS(!(any_of_elementwise<ApplyElementwiseFlags::nontrivial | ApplyElementwiseFlags::same_kind>)(NotEqual{}, a, b))
}

namespace em::Math
{
    // We use a free function instead of `operator<=>`, because we want to force the user to explicitly select the comparison flavor.
    // If you implement `<=>` in terms of this, you should replace `apply_elementwise()` with `apply_elementwise<ApplyElementwiseFlags::nontrivial | ApplyElementwiseFlags::same_kind>()`.
    template <typename A, typename B> requires(_adl_em_vec_allow_operator(VectorOps::CompareThreeWay{}, Meta::Tag<const A &>(), Meta::Tag<const B &>()))
    [[nodiscard]] EM_TINY constexpr auto comparee_three_way(const A &a, const B &b) EM_RETURNS(auto((any_of_elementwise)(VectorOps::CompareThreeWay{}, a, b).value))
}
