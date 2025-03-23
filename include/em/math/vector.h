#pragma once

#include "em/macros/meta/eval.h"
#include "em/macros/portable/always_inline.h"
#include "em/macros/portable/artificial.h"
#include "em/macros/portable/assume.h"
#include "em/macros/portable/if_consteval.h"
#include "em/macros/utils/cvref.h"
#include "em/math/type_shorthands.h"
#include "em/math/vector_operators.h"
#include "em/meta/cvref.h"

#include <utility>

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
        template <Meta::cvref_unqualified T>
        struct VecTraits
        {
            static constexpr int size = 1;
            using type = T;
        };
        template <typename T, int N>
        struct VecTraits<vec<T,N>>
        {
            static constexpr int size = N;
            using type = T;
        };
    }

    // If `T` is a vector (maybe cvref-qualified), returns its base type, otherwise returns just that cvref-unqualified type.
    template <typename T>
    using vec_base_t = typename detail::Vector::VecTraits<std::remove_cvref_t<T>>::type;

    // If `T` is a vector (maybe cvref-qualified), returns its base type with cvref copied from T. Otherwise returns exactly `T`.
    template <typename T>
    using vec_base_cvref_t = Meta::copy_cvref<T, vec_base_t<T>>;

    // If `T` is a vector (maybe cvref-qualified), returns its size. Otherwise returns 1.
    // If multiple arguments are specified, all of them must have the same size, otherwise causes a SFINAE error.
    template <typename T, typename ...P> requires ((detail::Vector::VecTraits<std::remove_cvref_t<T>>::size == detail::Vector::VecTraits<std::remove_cvref_t<P>>::size) && ...)
    static constexpr int vec_size = detail::Vector::VecTraits<std::remove_cvref_t<T>>::size;

    // Returns the size of all vectors (maybe cvref). If the sizes don't match, returns 0.
    // If there are only scalars, return 1. If the list is empty, returns 1.
    template <typename ...P>
    static constexpr int common_vec_size_or_zero =
        []{
            int r = 1;
            return ((vec_size<P> == 1 || vec_size<P> == r ? true : r == 1 ? (void(r = vec_size<P>), true) : false) && ...) ? r : 0;
        }();

    // Checks that all vectors in `P...` have the same size, ignoring non-vectors.
    // Returns true if there are no vectors, or if the list empty.
    template <typename ...P>
    concept have_common_vec_size = common_vec_size_or_zero<P...> > 0;

    // Returns the size of all vectors (maybe cvref). If the sizes don't match, triggers a SFINAE error.
    // If there are only scalars, return 1. If the list is empty, returns 1.
    template <typename ...P> requires have_common_vec_size<P...>
    static constexpr int common_vec_size = common_vec_size_or_zero<P...>;

    // Some concepts:

    template <typename T> concept vector_cvref = detail::Vector::VecTraits<std::remove_cvref_t<T>>::size > 1;
    template <typename T> concept vector = Meta::cvref_unqualified<T> && vector_cvref<T>;
    template <typename T> concept vector_or_scalar_cvref = vector_cvref<T> || scalar_cvref<T>;
    template <typename T> concept vector_or_scalar = vector<T> || scalar<T>;

    template <typename T> concept floating_point_vector_cvref = vector_cvref<T> && floating_point_scalar<vec_base_t<T>>;
    template <typename T> concept floating_point_vector = Meta::cvref_unqualified<T> && floating_point_vector_cvref<T>;
    template <typename T> concept floating_point_vector_or_scalar_cvref = floating_point_vector_cvref<T> || floating_point_scalar_cvref<T>;
    template <typename T> concept floating_point_vector_or_scalar = floating_point_vector<T> || floating_point_scalar<T>;

    template <typename T> concept signed_integral_vector_cvref = vector_cvref<T> && signed_integral_scalar<vec_base_t<T>>;
    template <typename T> concept signed_integral_vector = Meta::cvref_unqualified<T> && signed_integral_vector_cvref<T>;
    template <typename T> concept signed_integral_vector_or_scalar_cvref = signed_integral_vector_cvref<T> || signed_integral_scalar_cvref<T>;
    template <typename T> concept signed_integral_vector_or_scalar = signed_integral_vector<T> || signed_integral_scalar<T>;

    template <typename T> concept unsigned_integral_vector_cvref = vector_cvref<T> && unsigned_integral_scalar<vec_base_t<T>>;
    template <typename T> concept unsigned_integral_vector = Meta::cvref_unqualified<T> && unsigned_integral_vector_cvref<T>;
    template <typename T> concept unsigned_integral_vector_or_scalar_cvref = unsigned_integral_vector_cvref<T> || unsigned_integral_scalar_cvref<T>;
    template <typename T> concept unsigned_integral_vector_or_scalar = unsigned_integral_vector<T> || unsigned_integral_scalar<T>;

    template <typename T> concept integral_vector_cvref = vector_cvref<T> && integral_scalar<vec_base_t<T>>;
    template <typename T> concept integral_vector = Meta::cvref_unqualified<T> && integral_vector_cvref<T>;
    template <typename T> concept integral_vector_or_scalar_cvref = integral_vector_cvref<T> || integral_scalar_cvref<T>;
    template <typename T> concept integral_vector_or_scalar = integral_vector<T> || integral_scalar<T>;

    // Returns i-th element of a vector. If `v` is not a vector, returns it as is.
    // Prefer `apply_elementwise` to looping over `vec_elem`, because `apply_elementwise` supports more types (rects, etc).
    template <typename T>
    [[nodiscard]] constexpr auto &&vec_elem(int i, T &&v) noexcept
    {
        if constexpr (vector_cvref<T>)
            return EM_FWD(v)[i];
        else
            return v; // No forwarding, because it might be called several times on the same object.
    }

    namespace detail::Vector
    {
        // This base provides the x,y,z,w members for vectors. And also the elementwise constructor.
        template <typename T, int N, typename Derived> requires ValidSize<N>
        struct VectorMembers;

        #define DETAIL_EM_VEC(N, seq) EM_EVAL( \
            T EM_FOREACH_A(seq,(,))( EM_A0{} ); \
            [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL \
            constexpr VectorMembers() noexcept(std::is_nothrow_constructible_v<T>) {} \
            /* Construct elementwise: */ \
            /* This is intentionally not templated (with `explicit(can_safely_convert)`), because we almost always */ \
            /*   use the explicit notation anyway, and we want to disable narrowing conversions even in that case. */ \
            /* And it's easier to delegate the checking to the `-Wconversion` warnings than to do it ourselves. */ \
            [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL \
            constexpr VectorMembers EM_P( EM_FOREACH_A(seq,(,))( T EM_A0 ) ) noexcept(std::is_nothrow_move_constructible_v<T>) \
                : EM_FOREACH_A(seq,(,))( EM_A0 EM_P(std::move EM_P(EM_A0)) ) \
            {} \
            /* Fill with the same element: */ \
            [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL explicit \
            constexpr VectorMembers(T n) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_move_constructible_v<T>) \
                : EM_FOREACH_A(seq,(,))( EM_A0 EM_P(EM_A1(n)) ) \
            {} \
            /* Convert from a vector of another type: */ \
            template <typename U> requires(vec_size<U> == N) [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL explicit(!can_safely_convert<U, vec<T,N>>) \
            constexpr VectorMembers(U &&other) noexcept(std::is_nothrow_constructible_v<T, vec_base_cvref_t<U &&>>) requires std::is_constructible_v<T, vec_base_cvref_t<U>> \
                : EM_FOREACH_A(seq,(,))( EM_A0 EM_P(std::move EM_P(other.EM_A0)) ) \
            {} \
            /* Not using "deducing this" here because it prevents the use of this function in `EM_RETURNS()` in derived classes, */\
            /*   see: https://stackoverflow.com/q/79081096/2752075 */\
            EM_MAYBE_CONST_LR_LOOP( \
                /* Applies unary functor to each element, returns a new vector. */\
                [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto map(auto &&f) EM_QUAL EM_RETURNS EM_P(rebind_to<std::decay_t<decltype EM_P(f EM_P(EM_FWD_SELF.x))>, Derived> EM_P(EM_FOREACH_A(seq,(,))( f EM_P(EM_FWD_SELF.EM_A0) ))) \
                /* Calls a function with all elements as parameters. */\
                [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto apply(auto &&f) EM_QUAL EM_RETURNS EM_P(EM_FWD(f) EM_P(EM_FOREACH_A(seq,(,))( EM_FWD_SELF.EM_A0 ))) \
                /* Change the element type. */\
                template <Meta::cvref_unqualified U> requires(std::is_constructible_v<U, T>) \
                [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto to() EM_QUAL EM_RETURNS EM_P(vec<U,N> EM_P(EM_FOREACH_A(seq,(,))( EM_FWD_SELF.EM_A0 ))) \
            ) \
        )

        template <typename T, typename Derived>
        struct VectorMembers<T, 2, Derived>
        {
            DETAIL_EM_VEC(2, (x,)(y,std::move))

            EM_MAYBE_CONST_LR(
                // Reduces all elements over a binary function.
                [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto reduce(auto &&f) EM_QUAL EM_RETURNS EM_P(EM_FWD(f) EM_P(EM_FWD_SELF.x, EM_FWD_SELF.y))
            )
        };
        template <typename T, typename Derived>
        struct VectorMembers<T, 3, Derived>
        {
            DETAIL_EM_VEC(3, (x,)(y,)(z,std::move))

            EM_MAYBE_CONST_LR(
                // Reduces all elements over a binary function.
                [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto reduce(auto &&f) EM_QUAL EM_RETURNS EM_P(f EM_P(EM_FWD_SELF.x, f EM_P(EM_FWD_SELF.y, EM_FWD_SELF.z)))
            )
        };
        template <typename T, typename Derived>
        struct VectorMembers<T, 4, Derived>
        {
            DETAIL_EM_VEC(4, (x,)(y,)(z,)(w,std::move))

            EM_MAYBE_CONST_LR(
                // Reduces all elements over a binary function.
                [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto reduce(auto &&f) EM_QUAL EM_RETURNS EM_P(f EM_P(f EM_P(EM_FWD_SELF.x, EM_FWD_SELF.y), f EM_P(EM_FWD_SELF.z, EM_FWD_SELF.w)))
            )
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

        // RGBA-style member accessors.
        [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto &&r(this auto &&self) noexcept                   {return EM_FWD(self).x;}
        [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto &&g(this auto &&self) noexcept                   {return EM_FWD(self).y;}
        [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto &&b(this auto &&self) noexcept requires (N >= 3) {return EM_FWD(self).z;}
        [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto &&a(this auto &&self) noexcept requires (N >= 4) {return EM_FWD(self).w;}

        // Returns i-th element.
        [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto &&operator[](this auto &&self, int i) noexcept
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
        [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto sum() EM_RETURNS(this->reduce(Ops::Add{}))
        // The product of elements.
        [[nodiscard]] EM_ALWAYS_INLINE EM_ARTIFICIAL constexpr auto prod() EM_RETURNS(this->reduce(Ops::Mul{}))
    };

    // The obvious deduction guide, using `larger_t`.
    template <typename ...P>
    vec(P...) -> vec<larger_t<P...>, sizeof...(P)>;


    // Implement `apply_elementwise()` for vectors.
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
        requires (vector<A> || vector<B>) && have_common_vec_size<A, B>
        struct LargerType<A, B> {using type = vec<larger_t<vec_base_t<A>, vec_base_t<B>>, common_vec_size<A, B>>;};
    }
}

// `using namespace` this to import some common names.
namespace em
{
    namespace Math::Common
    {
        using Math::vec;
        EM_MATH_IMPORT_TYPE_SHORTHANDS_VEC(Math::,vec)
    }
    using namespace Math::Common;
}
