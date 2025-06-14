#pragma once

#include "em/macros/meta/codegen.h"
#include "em/macros/portable/assume.h"
#include "em/macros/portable/if_consteval.h"
#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/cvref.h"
#include "em/math/min_max.h"
#include "em/math/namespaces.h"
#include "em/math/rebind.h"
#include "em/math/scalar.h"
#include "em/math/type_shorthands.h"
#include "em/math/vector_operators.h"
#include "em/math/vector_traits.h"
#include "em/meta/common.h"

#include <utility>

// This header is self-sufficient for vectors. The overloaded operators for vectors are included here too.

namespace em::Math
{
    namespace detail::Vector
    {
        template <int N>
        concept ValidSize = N >= 2 && N <= 4;
    }

    // Define typedefs: `TvecN`, `Tvec<N>`, `vecN<T>`.
    EM_MATH_TYPE_SHORTHANDS_VEC(
        (template <Meta::cvref_unqualified T, int N> requires detail::Vector::ValidSize<N> struct),
        vec
    )

    namespace detail::Vector
    {
        // This base provides the x,y,z,w members for vectors. And also the elementwise constructor.
        template <typename T, int N, typename Derived> requires ValidSize<N>
        struct VectorMembers;

        #define DETAIL_EM_VEC(N, seq, reduce_) \
            T EM_CODEGEN(seq,(,), EM_1{} ); \
            [[nodiscard]] EM_TINY \
            constexpr VectorMembers() noexcept(std::is_nothrow_constructible_v<T>) {} \
            /* Construct elementwise: */ \
            /* This is intentionally not templated (with `explicit(can_safely_convert)`), because we almost always */ \
            /*   use the explicit notation anyway, and we want to disable narrowing conversions even in that case. */ \
            /* And it's easier to delegate the checking to the `-Wconversion` warnings than to do it ourselves. */ \
            [[nodiscard]] EM_TINY \
            constexpr VectorMembers( EM_CODEGEN(seq,(,), T EM_1 ) ) noexcept(std::is_nothrow_move_constructible_v<T>) \
                : EM_CODEGEN(seq,(,), EM_1 EM_P(std::move EM_P(EM_1)) ) \
            {} \
            /* Fill with the same element: */ \
            [[nodiscard]] EM_TINY explicit \
            constexpr VectorMembers(T n) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_move_constructible_v<T>) \
                : EM_CODEGEN(seq,(,), EM_1 EM_P(EM_3_OPT(n)) ) \
            {} \
            /* Convert from a vector of another type: */ \
            template <typename U> requires(vec_size<U> == N) [[nodiscard]] EM_TINY explicit(!can_safely_convert<U, vec<T,N>>) \
            constexpr VectorMembers(U &&other) noexcept(std::is_nothrow_constructible_v<T, vec_base_cvref_t<U &&>>) requires std::is_constructible_v<T, vec_base_cvref_t<U>> \
                : EM_CODEGEN(seq,(,), EM_1 EM_P(T EM_P(std::forward_like<U> EM_P(other.EM_1))) ) \
            {} \
            /* Not using "deducing this" here because it prevents the use of this function in `EM_RETURNS()` in derived classes, */\
            /*   see: https://stackoverflow.com/q/79081096/2752075 */\
            EM_MAYBE_CONST_LR( \
                /* Applies unary functor to each element, returns a new vector. */\
                [[nodiscard]] EM_TINY constexpr auto map(auto &&f) EM_QUAL EM_RETURNS EM_P(rebind_to<std::decay_t<decltype EM_P(f EM_P(EM_FWD_SELF.x))>, Derived> EM_P(EM_CODEGEN(seq,(,), f EM_E(EM_LP EM_FWD_SELF).EM_1 EM_E(EM_RP) ))) \
                /* Calls a function with all elements as parameters. */\
                [[nodiscard]] EM_TINY constexpr auto apply(auto &&f) EM_QUAL EM_RETURNS EM_P(EM_FWD(f) EM_P(EM_CODEGEN(seq,(,), EM_E(EM_FWD_SELF).EM_1 ))) \
                /* Change the element type. */\
                template <Meta::cvref_unqualified U> requires(std::is_constructible_v<U, T>) \
                [[nodiscard]] EM_TINY constexpr auto to() EM_QUAL EM_RETURNS EM_P(vec<U,N> EM_P(EM_CODEGEN(seq,(,), U EM_E(EM_LP EM_FWD_SELF).EM_1 EM_E(EM_RP) ))) \
                /* Reduces all elements over a binary function. */\
                [[nodiscard]] EM_TINY constexpr auto reduce(auto &&f) EM_QUAL EM_RETURNS EM_P( EM_UNWRAP_CODE(reduce_) )\
            ) \
            /* RGBA-style member accessors. They are here instead of `Vector` because disabling them with `requires` */\
            /* still shows them in the code completion, and that looks a big ugly. */\
            EM_CODEGEN(seq,, \
                [[nodiscard]] EM_TINY constexpr auto &&EM_2(this auto &&self) noexcept {return EM_FWD(self).EM_1;} \
            )

        template <typename T, typename Derived>
        struct VectorMembers<T, 2, Derived>
        {
            DETAIL_EM_VEC(2, (x,r)(y,g,std::move), (EM_FWD(f) EM_P(EM_FWD_SELF.x, EM_FWD_SELF.y)))
        };
        template <typename T, typename Derived>
        struct VectorMembers<T, 3, Derived>
        {
            DETAIL_EM_VEC(3, (x,r)(y,g)(z,b,std::move), (f EM_P(EM_FWD_SELF.x, f EM_P(EM_FWD_SELF.y, EM_FWD_SELF.z))))
        };
        template <typename T, typename Derived>
        struct VectorMembers<T, 4, Derived>
        {
            DETAIL_EM_VEC(4, (x,r)(y,g)(z,b)(w,a,std::move), (f EM_P(f EM_P(EM_FWD_SELF.x, EM_FWD_SELF.y), f EM_P(EM_FWD_SELF.z, EM_FWD_SELF.w))))
        };
    }

    template <Meta::cvref_unqualified T, int N> requires detail::Vector::ValidSize<N>
    struct vec : VectorOps::EnableVectorOps<vec<T,N>>, detail::Vector::VectorMembers<T, N, vec<T,N>>
    {
        static constexpr int dims = N;
        using type = T;

        // T x{}, y{}...;
        // vec() {}
        // vec(T x, T y...) : x(std::move(x)), y(std::move(y))... {}
        // explicit vec(T n) : x(n), y(std::move(n))... {}
        // explicit(can_safely_convert) vec(vec<N,T> n) : x(n), y(std::move(n))... {}
        using detail::Vector::VectorMembers<T, N, vec<T,N>>::VectorMembers;

        // map(f) // apply unary functor, return a new vector
        // apply(f) // apply N-ary functor, return the result
        // reduce(f) // reduce over binary functor
        // r(),g(),b(),a() // member accessors with different names

        // Returns i-th element.
        [[nodiscard]] EM_TINY constexpr auto &&operator[](this auto &&self, int i) noexcept
        {
            EM_ASSUME(i >= 0 && i < N);
            EM_IF_CONSTEVAL
            {
                                      if (i == 0) return EM_FWD(self).x;
                                      if (i == 1) return EM_FWD(self).y;
                if constexpr (N >= 3) if (i == 2) return EM_FWD(self).z;
                if constexpr (N >= 4) if (i == 3) return EM_FWD(self).w;
                std::unreachable();
            }
            else
            {
                // This is technically UB, but helps MSVC to optimize the code. Clang and GCC are fine either way.
                return std::forward_like<decltype(self)>((&self.x)[i]);
            }
        }

        // The sum of elements.
        [[nodiscard]] EM_TINY constexpr auto sum() EM_SOFT_RETURNS(this->reduce(Ops::Add{}))
        // The product of elements.
        [[nodiscard]] EM_TINY constexpr auto prod() EM_SOFT_RETURNS(this->reduce(Ops::Mul{}))

        // Min/max of all elements.
        [[nodiscard]] EM_TINY constexpr auto min() EM_SOFT_RETURNS(this->apply(Math::min))
        [[nodiscard]] EM_TINY constexpr auto max() EM_SOFT_RETURNS(this->apply(Math::max))

        // Convert to shorter or longer vectors. When converting to a longer vector, either pass the missing components or they will be zeroed.
        [[nodiscard]] EM_TINY constexpr vec2<T> to_vec2(this auto &&self) requires (N > 2) {return vec2<T>(EM_FWD(self).x, EM_FWD(self).y);}
        [[nodiscard]] EM_TINY constexpr vec3<T> to_vec3(this auto &&self) requires (N > 3) {return vec3<T>(EM_FWD(self).x, EM_FWD(self).y, EM_FWD(self).z);}
        [[nodiscard]] EM_TINY constexpr vec3<T> to_vec3(this auto &&self, T z     ) requires (N == 2) {return vec3<T>(EM_FWD(self).x, EM_FWD(self).y, std::move(z));}
        [[nodiscard]] EM_TINY constexpr vec4<T> to_vec4(this auto &&self, T z, T w) requires (N == 2) {return vec4<T>(EM_FWD(self).x, EM_FWD(self).y, std::move(z),   std::move(w));}
        [[nodiscard]] EM_TINY constexpr vec4<T> to_vec4(this auto &&self,      T w) requires (N == 3) {return vec4<T>(EM_FWD(self).x, EM_FWD(self).y, EM_FWD(self).z, std::move(w));}
        [[nodiscard]] EM_TINY constexpr vec3<T> to_vec3(this auto &&self) requires (N == 2) {return vec3<T>(EM_FWD(self).x, EM_FWD(self).y, T{});}
        [[nodiscard]] EM_TINY constexpr vec4<T> to_vec4(this auto &&self) requires (N == 2) {return vec4<T>(EM_FWD(self).x, EM_FWD(self).y, T{},            (OneIfScalar<T>)());}
        [[nodiscard]] EM_TINY constexpr vec4<T> to_vec4(this auto &&self) requires (N == 3) {return vec4<T>(EM_FWD(self).x, EM_FWD(self).y, EM_FWD(self).z, (OneIfScalar<T>)());}
    };

    // The obvious deduction guide, using `larger_t`.
    template <typename ...P>
    vec(P...) -> vec<larger_t<P...>, sizeof...(P)>;


    // Specialize `VectorTraits` for vectors.
    namespace Customize
    {
        template <typename T, int N>
        struct VectorTraits<vec<T, N>>
        {
            static constexpr int size = N;
            using type = T;

            [[nodiscard]] static constexpr auto &&GetElem(int i, Meta::same_or_derived_from_ignoring_cvref<vec<T, N>> auto &&v)
            {
                return EM_FWD(v)[i];
            }
        };
    }

    // Implement `apply_elementwise()` for vectors.
    // This probably could be generalized and moved to `vector_traits.h`, but it needs some more thought (e.g. what return type should `ivec2 + ImVec2` have).
    namespace detail::Vector
    {
        template <bool SameKind>
        struct MaybeSameVecSize
        {
            template <typename ...P>
            requires requires{common_vec_size<P...>;}
            static constexpr int value = common_vec_size<P...>;
        };
        template <>
        struct MaybeSameVecSize<true>
        {
            template <typename ...P>
            requires requires{vec_size<P...>;}
            static constexpr int value = vec_size<P...>;
        };

        // Implement `apply_elementwise()` for vectors.
        // Using the oldschool SFINAE to make sure it gets checked before `EM_RETURNS`, which could potentially be circular.
        // Not forwarding `func` on the latest iteration to match `CanApplyElementwiseVector`, which would be harder to write for that case.
        template <bool SameKind, typename F, typename ...P> requires (MaybeSameVecSize<SameKind>::template value<P...> == 2) constexpr auto _adl_em_apply_elementwise(F &&func, P &&... params) EM_RETURNS(vec{std::invoke(func, (vec_elem)(0, EM_FWD(params))...), std::invoke(func, (vec_elem)(1, EM_FWD(params))...)})
        template <bool SameKind, typename F, typename ...P> requires (MaybeSameVecSize<SameKind>::template value<P...> == 3) constexpr auto _adl_em_apply_elementwise(F &&func, P &&... params) EM_RETURNS(vec{std::invoke(func, (vec_elem)(0, EM_FWD(params))...), std::invoke(func, (vec_elem)(1, EM_FWD(params))...), std::invoke(func, (vec_elem)(2, EM_FWD(params))...)})
        template <bool SameKind, typename F, typename ...P> requires (MaybeSameVecSize<SameKind>::template value<P...> == 4) constexpr auto _adl_em_apply_elementwise(F &&func, P &&... params) EM_RETURNS(vec{std::invoke(func, (vec_elem)(0, EM_FWD(params))...), std::invoke(func, (vec_elem)(1, EM_FWD(params))...), std::invoke(func, (vec_elem)(2, EM_FWD(params))...), std::invoke(func, (vec_elem)(3, EM_FWD(params))...)})

        // Implement `any_of_elementwise()` for vectors.
        template <bool SameKind, typename F, typename ...P> requires (MaybeSameVecSize<SameKind>::template value<P...> == 2) constexpr auto _adl_em_any_of_elementwise(F &&func, P &&... params) noexcept(noexcept(auto(std::invoke(func, (vec_elem)(0, EM_FWD(params))...)))) -> decltype(auto(std::invoke(func, (vec_elem)(0, EM_FWD(params))...))) {if (auto d = std::invoke(func, (vec_elem)(0, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, (vec_elem)(1, EM_FWD(params))...)) return d; return {};}
        template <bool SameKind, typename F, typename ...P> requires (MaybeSameVecSize<SameKind>::template value<P...> == 3) constexpr auto _adl_em_any_of_elementwise(F &&func, P &&... params) noexcept(noexcept(auto(std::invoke(func, (vec_elem)(0, EM_FWD(params))...)))) -> decltype(auto(std::invoke(func, (vec_elem)(0, EM_FWD(params))...))) {if (auto d = std::invoke(func, (vec_elem)(0, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, (vec_elem)(1, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, (vec_elem)(2, EM_FWD(params))...)) return d; return {};}
        template <bool SameKind, typename F, typename ...P> requires (MaybeSameVecSize<SameKind>::template value<P...> == 4) constexpr auto _adl_em_any_of_elementwise(F &&func, P &&... params) noexcept(noexcept(auto(std::invoke(func, (vec_elem)(0, EM_FWD(params))...)))) -> decltype(auto(std::invoke(func, (vec_elem)(0, EM_FWD(params))...))) {if (auto d = std::invoke(func, (vec_elem)(0, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, (vec_elem)(1, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, (vec_elem)(2, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, (vec_elem)(3, EM_FWD(params))...)) return d; return {};}
    }

    // Implement `larger_t` logic for vectors.
    namespace Customize
    {
        template <Meta::cvref_unqualified A, Meta::cvref_unqualified B>
        requires (vector<A> || vector<B>) && have_common_vec_size<A, B> && have_larger_type<vec_base_t<A>, vec_base_t<B>>
        struct LargerType<A, B> {using type = vec<larger_t<vec_base_t<A>, vec_base_t<B>>, common_vec_size<A, B>>;};
    }
}

// Expose `vec` and its typedefs into the `Common` namespace.
namespace em::Math::inline Common
{
    using Math::vec;
    EM_MATH_IMPORT_TYPE_SHORTHANDS_VEC(Math::,vec)
}
