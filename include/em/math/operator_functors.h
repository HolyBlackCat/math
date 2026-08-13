#pragma once

#include "em/macros/meta/common.h"
#include "em/macros/portable/tiny_func.h"
#include "em/math/larger_type.h"

#include <type_traits>

// Functors for all math operators, with some fixes, such as:
// * Don't promote the return type for scalars.
// * No UB when multiplying large unsigned floats.

namespace em::Math::Ops
{
    // Unary operators, except for the boolean `!` which shouldn't be overloaded.
    #define EM_MATH_OPS_UNARY(unary) \
        unary(Pos, +) \
        unary(Neg, -) \
        unary(Compl, ~)

    // Binary operators, except for the boolean `&&`/`||` which shouldn't be overloaded, and except for comparison operators.
    #define EM_MATH_OPS_BINARY(binary) \
        binary(Add   , +        ) binary(Sub   , -        ) \
        binary(Mul   , *        ) binary(Div   , /        ) binary(Mod   , % ) \
        binary(BitAnd, &        ) binary(BitOr , |        ) binary(BitXor, ^ ) \
        binary(Lshift, <<, Shift) binary(Rshift, >>, Shift)

    namespace detail
    {
        template <typename T>
        concept BuiltinScalar = std::is_arithmetic_v<std::remove_cvref_t<T>>;

        // Those scalars get promoted by the standard operators.
        template <typename T>
        concept BuiltinScalarSmall = BuiltinScalar<T> && sizeof(T) < sizeof(int);

        template <typename T, typename U>
        concept BothBuiltinScalarsAndAtLeastOneSmall = BuiltinScalar<T> && BuiltinScalar<U> && (BuiltinScalarSmall<T> || BuiltinScalarSmall<U>);
    }

    #define DETAIL_EM_X(name_, op_) \
        struct name_ \
        { \
            template <typename T> \
            [[nodiscard]] EM_TINY static constexpr auto operator()(T &&value) \
            noexcept(noexcept(auto(op_ decltype(value)(value)))) \
            requires requires{auto(op_ decltype(value)(value));} \
            { \
                if constexpr (detail::BuiltinScalarSmall<T>) \
                    return std::remove_cvref_t<T>(op_ value); \
                else \
                    return op_ decltype(value)(value); \
            } \
        };
    EM_MATH_OPS_UNARY(DETAIL_EM_X)
    #undef DETAIL_EM_X

    namespace detail
    {
        // Those disable operators on builtin scalars if they involve weird conversions.

        template <typename T, typename U>
        concept AllowBinaryOperator = !std::is_arithmetic_v<std::remove_cvref_t<T>> || have_larger_type<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

        template <typename T, typename U>
        concept AllowBinaryOperatorAssign = !std::is_arithmetic_v<std::remove_cvref_t<T>> || can_safely_convert_to<std::remove_cvref_t<U>, std::remove_cvref_t<T>>;
    }

    #define DETAIL_EM_X(name_, op_, ...) \
        struct name_ \
        { \
            template <typename T, typename U> EM_CAT(DETAIL_EM_X2_, __VA_ARGS__)() \
            [[nodiscard]] EM_TINY static constexpr auto operator()(T &&t, U &&u) \
            noexcept(noexcept(auto(decltype(t)(t) op_ decltype(u)(u)))) \
            requires requires{auto(decltype(t)(t) op_ decltype(u)(u));} \
            { \
                if constexpr (detail::BothBuiltinScalarsAndAtLeastOneSmall<T, U>) \
                { \
                    EM_CAT(DETAIL_EM_X_, __VA_ARGS__)(op_); \
                } \
                else \
                { \
                    return decltype(t)(t) op_ decltype(u)(u); \
                } \
            } \
        }; \
        struct EM_CAT(name_, Assign) \
        { \
            template <typename T, typename U> EM_CAT(DETAIL_EM_X3_, __VA_ARGS__)() \
            /* Force lvalue lhs here, to prevent accidantal errors. */\
            [[nodiscard]] EM_TINY static constexpr T &operator()(T &t, U &&u) \
            noexcept(noexcept(t EM_CAT(op_,=) decltype(u)(u))) \
            requires requires{auto(t EM_CAT(op_,=) decltype(u)(u));} \
            { \
                if constexpr (detail::BothBuiltinScalarsAndAtLeastOneSmall<T, U>) \
                { \
                    using LP = Meta::Scalars::PromoteWithSameSign<larger_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>>; \
                    t = std::remove_cvref_t<T>(LP(t) op_ LP(u)); \
                } \
                else \
                { \
                    t EM_CAT(op_,=) decltype(u)(u); \
                } \
                return t; \
            } \
        };
    #define DETAIL_EM_X_(op_) \
        using L = larger_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>; \
        using LP = Meta::Scalars::PromoteWithSameSign<L>; \
        return L(LP(t) op_ LP(u))
    #define DETAIL_EM_X_Shift(op_) \
        using L = std::remove_cvref_t<T>; \
        using LP = Meta::Scalars::PromoteWithSameSign<L>; \
        return L(LP(t) op_ u)
    #define DETAIL_EM_X2_() requires detail::AllowBinaryOperator<T, U>
    #define DETAIL_EM_X2_Shift()
    #define DETAIL_EM_X3_() requires detail::AllowBinaryOperatorAssign<T, U>
    #define DETAIL_EM_X3_Shift()
    EM_MATH_OPS_BINARY(DETAIL_EM_X)
    #undef DETAIL_EM_X
    #undef DETAIL_EM_X_
    #undef DETAIL_EM_X_Shift
    #undef DETAIL_EM_X2_
    #undef DETAIL_EM_X2_Shift
    #undef DETAIL_EM_X3_
    #undef DETAIL_EM_X3_Shift
}
