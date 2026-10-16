#pragma once

#include "em/macros/utils/forward.h" // IWYU pragma: keep. We use `EM_FWD` in this file, don't know why it warns.
#include "em/macros/utils/functors.h"
#include "em/macros/utils/returns.h"
#include "em/macros/utils/sfinae.h"
#include "em/math/larger_type.h"
#include "em/math/scalar.h"
#include "em/meta/common.h"
#include "em/meta/functional.h"

#include <type_traits>

// Everything here is designed to in theory accept third-party vector types.

namespace em::Math
{
    namespace Customize
    {
        template <Meta::cvref_unqualified T>
        struct VectorTraits
        {
            // The primary template is for scalars (and generally all non-vector types).
            // Specialize this for vector types.

            // Additional requirements for `T`:
            // 1. Must be constructible from `size` arguments of type `type`.

            static constexpr int size = 1;
            using type = T;

            // This function must not forward if it's going to return the same element for different indices, like we're doing here for scalars.
            [[nodiscard]] static constexpr auto &&GetElem(int i, Meta::same_or_derived_from_ignoring_cvref<T> auto &&v) noexcept
            {
                (void)i;
                return v; // Intentionally no forwarding.
            }

            // Given `vecN<U>`, returns `vecN<T>`. Makes no sense for scalars.
            // This should return the same type if `T == type`. But otherwise it doesn't have to roundtrip. If your vector is not templated, you can just return the default `vec<...>`.
            //   template <typename T> using change_base = ...;
            // Given `vec<T,M>`, returns `vec<T,N>`. Makes no sense for scalars.
            // You don't need to support `N == 1`, we do this ourselves.
            // This should return the same type if `N == size`. But otherwise it doesn't have to roundtrip. If your vector is not templated, you can just return the default `vec<...>`.
            //   template <int N> using change_size = ...;
        };
    }

    // If `T` is a vector (maybe cvref-qualified), returns its base type, otherwise returns just that cvref-unqualified type.
    template <typename T>
    using vec_base_t = typename Customize::VectorTraits<std::remove_cvref_t<T>>::type;

    // If `T` is a vector (maybe cvref-qualified), returns its base type with cvref copied from T. Otherwise returns exactly `T`.
    template <typename T>
    using vec_base_cvref_t = Meta::copy_cvref<T, vec_base_t<T>>;

    // Returns true if all elements in `T, P...` are vectors with the same size (or all non-vectors).
    // Compare to `have_common_vec_size`.
    template <typename T, typename ...P>
    concept have_same_vec_size = ((Customize::VectorTraits<std::remove_cvref_t<T>>::size == Customize::VectorTraits<std::remove_cvref_t<P>>::size) && ...);

    // If `T` is a vector (maybe cvref-qualified), returns its size. Otherwise returns 1.
    // If multiple arguments are specified, all of them must have the exact same size, otherwise causes a SFINAE error.
    template <typename T, typename ...P> requires have_same_vec_size<T, P...>
    static constexpr int vec_size = Customize::VectorTraits<std::remove_cvref_t<T>>::size;

    // Returns the size of all vectors (maybe cvref). If the sizes don't match, returns 0.
    // If there are only scalars, return 1. If the list is empty, returns 1.
    // Unlike the plain `vec_size`, this ignores scalars.
    template <typename ...P>
    static constexpr int common_vec_size_or_zero =
        []{
            int r = 1;
            return ((vec_size<P> == 1 || vec_size<P> == r ? true : r == 1 ? (void(r = vec_size<P>), true) : false) && ...) ? r : 0;
        }();

    // Checks that all vectors in `P...` have the same size, ignoring non-vectors.
    // Returns true if there are no vectors, or if the list empty.
    // Unlike the plain `vec_size` and `have_same_vec_size`, this ignores scalars.
    template <typename ...P>
    concept have_common_vec_size = common_vec_size_or_zero<P...> != 0;

    // Returns the size of all vectors (maybe cvref). If the sizes don't match, triggers a SFINAE error.
    // If there are only scalars, return 1. If the list is empty, returns 1.
    // Unlike the plain `vec_size`, this ignores scalars.
    template <typename ...P> requires have_common_vec_size<P...>
    static constexpr int common_vec_size = common_vec_size_or_zero<P...>;


    // Dispatches between `vec_size<T, P...>` and `common_vec_size<T, P...>` based on the template parameter `isCommon`.
    template <bool IsCommon, typename T, typename ...P>
    static constexpr int maybe_common_vec_size = -1; // Unused primary template.
    template <typename T, typename ...P> requires have_same_vec_size<T, P...>
    static constexpr int maybe_common_vec_size<false, T, P...> = vec_size<T, P...>;
    template <typename T, typename ...P> requires have_common_vec_size<T, P...>
    static constexpr int maybe_common_vec_size<true, T, P...> = common_vec_size<T, P...>;


    // Some concepts:
    // Remember that vectors can hold more things than just scalars, so don't overuse `vector_or_scalar`.

    template <typename T> concept vector_cvref = Customize::VectorTraits<std::remove_cvref_t<T>>::size > 1;
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


    // Modifying vector properties:

    namespace detail::VecTraits
    {
        template <typename V, int N>
        struct ChangeSize;
        template <typename V>
        struct ChangeSize<V, 1> {using type = vec_base_t<V>;};
        // This has to be in a specialization to have a constraint, which we need to make `change_vec_size` SFINAE-friendly.
        template <typename V, int N> requires (N > 1) && requires{typename Customize::VectorTraits<V>::template change_size<N>;}
        struct ChangeSize<V, N> {using type = typename Customize::VectorTraits<V>::template change_size<N>;};
    }

    // Given `V == vec<U,N>`, returns `vec<T,N>` (or similarly with any class implementing vector traits). For non-vectors just returns `T`.
    // The same operation is applied to all vectors in `V...`, and the resulting types must all be the same, otherwise a SFINAE error is triggered.
    template <typename T, vector ...V> requires Meta::same_as_all<typename Customize::VectorTraits<V>::template change_base<T>...>
    using change_vec_base = typename Customize::VectorTraits<V...[0]>::template change_base<T>;

    // Given `V == vec<T,M>`, returns `vec<T,N>`.
    template <vector V, int N> requires requires{typename detail::VecTraits::ChangeSize<V, N>::type;}
    using change_vec_size = typename detail::VecTraits::ChangeSize<V, N>::type;


    namespace detail::VecTraits
    {
        // struct IsVectorPred
        // {
        //     template <typename T>
        //     using type = std::bool_constant<vector<T>>;
        // };

        // template <typename T, typename ...P>
        // struct ChangeCommonVecBase {};
        // // We intentionally don't fall back to anything if the predicate fails.
        // template <typename T, typename V0, typename ...V, typename ...S> requires requires{typename change_vec_base<T, V0, V...>;}
        // struct ChangeCommonVecBase<T, Meta::TypePair<Meta::TypeList<V0, V...>, Meta::TypeList<S...>>> {using type = change_vec_base<T, V0, V...>;};
        // // I'm not sure if rejecting empty lists is useful here. Doing it just in case.
        // template <typename T, typename S0, typename ...S>
        // struct ChangeCommonVecBase<T, Meta::TypePair<Meta::TypeList<>, Meta::TypeList<S0, S...>>> {using type = T;}; // `T` is same as `change_vec_base<T, S0, S...>` here.

        template <typename R, typename T, typename ...P>
        struct ChangeCommonVecBase {};
        template <typename R0, typename ...R, typename T> requires requires{typename change_vec_base<T, R0, R...>;}
        struct ChangeCommonVecBase<Meta::TypeList<R0, R...>, T> {using type = change_vec_base<T, R0, R...>;};
        template <typename ...R, typename T, typename P0, typename ...P> requires vector<P0>
        struct ChangeCommonVecBase<Meta::TypeList<R...>, T, P0, P...> : ChangeCommonVecBase<Meta::TypeList<R..., P0>, T, P...> {};
        template <typename ...R, typename T, typename P0, typename ...P> requires(!vector<P0>)
        struct ChangeCommonVecBase<Meta::TypeList<R...>, T, P0, P...> : ChangeCommonVecBase<Meta::TypeList<R...>, T, P...> {};
    }

    // Similar to `change_vec_base`. If there's at least one vector in `V...`, calls `change_vec_base<...>` with all vectors in `V...`, ignoring non-vectors.
    // If there are no vectors in `V`, returns `T`.
    // `V...` must not be empty.
    template <typename T, Meta::cvref_unqualified ...V>
    using change_common_vec_base = typename detail::VecTraits::ChangeCommonVecBase<Meta::TypeList<>, T, V...>::type;


    // Some primitive functions to operate on vectors:
    // Everything more complex sits in `functions.h`.

    // Returns i-th element of a vector. If `v` is not a vector, returns it as is (a reference to it).
    // Prefer `apply_elementwise` to looping over `vec_elem`, because `apply_elementwise` supports more types (rects, etc).
    // NOTE: No `vector_or_scalar_cvref` to allow arbitrary types.
    EM_SIMPLE_FUNCTOR(
        vec_elem,
        (template <typename T>),
        (int i, T &&v) EM_RETURNS( Customize::VectorTraits<std::remove_cvref_t<T>>::GetElem(i, EM_FWD(v)) )
    )

    // There is no `vec_map()` here because that's just `apply_elementwise()`.

    // Reduces vector `v` over a binary function `func`, in an unspecified order.
    // If `v` is not a vector, returns it as is (a reference to it).
    // NOTE: No `vector_or_scalar_cvref` to allow arbitrary types.
    // We hardcode reductions for small vectors and don't generalize this, for simplicity.
    EM_SIMPLE_FUNCTOR(
        vec_reduce,
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 1)),
        (T &&v, auto &&/*func*/) EM_RETURNS( EM_FWD(v) )
        EM_OVERLOAD
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 2)),
        (T &&v, auto &&func) EM_RETURNS( EM_FWD(func)(
            vec_elem(0, EM_FWD(v)),
            vec_elem(1, EM_FWD(v))
        ) )
        EM_OVERLOAD
        // Not forwarding `func` since it's used more than once here. I know the final call happens last, but I'm not sure the functor to call there is evaluated last.
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 3)),
        (T &&v, auto &&func) EM_RETURNS( func(
            vec_elem(0, EM_FWD(v)),
            func(
                vec_elem(1, EM_FWD(v)),
                vec_elem(2, EM_FWD(v))
            )
        ) )
        EM_OVERLOAD
        // Not forwarding `func` since it's used more than once here. I know the final call happens last, but I'm not sure the functor to call there is evaluated last.
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 4)),
        (T &&v, auto &&func) EM_RETURNS( func(
            func(
                vec_elem(0, EM_FWD(v)),
                vec_elem(1, EM_FWD(v))
            ),
            func(
                vec_elem(2, EM_FWD(v)),
                vec_elem(3, EM_FWD(v))
            )
        ) )
    )

    // Calls a function with elements of vector `v` as arguments.
    // If `v` is not a vector, then the function is called with it as the only argument.
    // NOTE: No `vector_or_scalar_cvref` to allow arbitrary types.
    // We hardcode this for small vectors and don't generalize this, for simplicity.
    EM_SIMPLE_FUNCTOR(
        vec_apply,
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 1)),
        (T &&v, auto &&func) EM_RETURNS( EM_FWD(func)(EM_FWD(v)) )
        EM_OVERLOAD
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 2)),
        (T &&v, auto &&func) EM_RETURNS( EM_FWD(func)(vec_elem(0, EM_FWD(v)), vec_elem(1, EM_FWD(v))) )
        EM_OVERLOAD
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 3)),
        (T &&v, auto &&func) EM_RETURNS( EM_FWD(func)(vec_elem(0, EM_FWD(v)), vec_elem(1, EM_FWD(v)), vec_elem(2, EM_FWD(v))) )
        EM_OVERLOAD
        (template <typename T> requires (Customize::VectorTraits<std::remove_cvref_t<T>>::size == 4)),
        (T &&v, auto &&func) EM_RETURNS( EM_FWD(func)(vec_elem(0, EM_FWD(v)), vec_elem(1, EM_FWD(v)), vec_elem(2, EM_FWD(v)), vec_elem(3, EM_FWD(v))) )
    )

    // Usage: `vec_make_of_same_size<V...>(args...)`.
    // Deduces the vector size (and the specific vector template to use) from `V...`
    //   using `change_common_vec_base` (so if there's at least one vector, ignores all non-vectors), ignoring cvref.
    // Deduces the vector base type from `args...`, which must all have the same type ignoring cvref.
    // Then constructs a vector of the resulting type from `args...`.
    EM_SIMPLE_FUNCTOR_EXT(
        vec_make_of_same_size,
        (template <typename ...V> requires(sizeof...(V) > 0)), (EM_1<V...>),
        (
            template <
                typename ...P,
                EM_CHECK_COND(Meta::same_as_all<std::remove_cvref_t<P>...>),
                typename L = change_common_vec_base<std::remove_cvref_t<P...[0]>, std::remove_cvref_t<V>...>
            >
            requires(sizeof...(P) == vec_size<L>)
        ),
        (P &&... params) EM_RETURNS( L(EM_FWD(params)...) )
    )

    // Implement `larger_t` logic for vectors.
    namespace Customize
    {
        template <Meta::cvref_unqualified ...P>
        requires CustomLargerType<P...> && (vector<P> || ...) && have_common_vec_size<P...> && have_larger_type<vec_base_t<P>...>
        struct LargerType<P...> {using type = change_common_vec_base<larger_t<vec_base_t<P>...>, P...>;};
    }

    // Implement apply-elementwise for vectors:

    // Using the oldschool SFINAE to make sure it gets checked before `EM_RETURNS`, which could potentially be circular.
    // Not forwarding `func` on the latest iteration to match `CanApplyElementwiseVector`, which would be harder to write for that case.
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 2) constexpr auto _adl_em_apply_elementwise(std::nullptr_t, F &&func, P &&... params) EM_RETURNS(vec_make_of_same_size<P...>(std::invoke(func, vec_elem(0, EM_FWD(params))...), std::invoke(func, vec_elem(1, EM_FWD(params))...)))
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 3) constexpr auto _adl_em_apply_elementwise(std::nullptr_t, F &&func, P &&... params) EM_RETURNS(vec_make_of_same_size<P...>(std::invoke(func, vec_elem(0, EM_FWD(params))...), std::invoke(func, vec_elem(1, EM_FWD(params))...), std::invoke(func, vec_elem(2, EM_FWD(params))...)))
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 4) constexpr auto _adl_em_apply_elementwise(std::nullptr_t, F &&func, P &&... params) EM_RETURNS(vec_make_of_same_size<P...>(std::invoke(func, vec_elem(0, EM_FWD(params))...), std::invoke(func, vec_elem(1, EM_FWD(params))...), std::invoke(func, vec_elem(2, EM_FWD(params))...), std::invoke(func, vec_elem(3, EM_FWD(params))...)))
    // Same for `void` return type:
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 2) constexpr auto _adl_em_apply_elementwise(std::nullptr_t, F &&func, P &&... params) EM_RETURNS(Meta::invoke_void(func, vec_elem(0, EM_FWD(params))...), Meta::invoke_void(func, vec_elem(1, EM_FWD(params))...))
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 3) constexpr auto _adl_em_apply_elementwise(std::nullptr_t, F &&func, P &&... params) EM_RETURNS(Meta::invoke_void(func, vec_elem(0, EM_FWD(params))...), Meta::invoke_void(func, vec_elem(1, EM_FWD(params))...), Meta::invoke_void(func, vec_elem(2, EM_FWD(params))...))
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 4) constexpr auto _adl_em_apply_elementwise(std::nullptr_t, F &&func, P &&... params) EM_RETURNS(Meta::invoke_void(func, vec_elem(0, EM_FWD(params))...), Meta::invoke_void(func, vec_elem(1, EM_FWD(params))...), Meta::invoke_void(func, vec_elem(2, EM_FWD(params))...), Meta::invoke_void(func, vec_elem(3, EM_FWD(params))...))

    // Implement `any_of_elementwise()` for vectors.
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 2) constexpr auto _adl_em_any_of_elementwise(std::nullptr_t, F &&func, P &&... params) noexcept(noexcept(auto(std::invoke(func, vec_elem(0, EM_FWD(params))...)))) -> decltype(auto(std::invoke(func, vec_elem(0, EM_FWD(params))...))) {if (auto d = std::invoke(func, vec_elem(0, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, vec_elem(1, EM_FWD(params))...)) return d; return {};}
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 3) constexpr auto _adl_em_any_of_elementwise(std::nullptr_t, F &&func, P &&... params) noexcept(noexcept(auto(std::invoke(func, vec_elem(0, EM_FWD(params))...)))) -> decltype(auto(std::invoke(func, vec_elem(0, EM_FWD(params))...))) {if (auto d = std::invoke(func, vec_elem(0, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, vec_elem(1, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, vec_elem(2, EM_FWD(params))...)) return d; return {};}
    template <bool SameKind, typename F, typename ...P> requires (maybe_common_vec_size<!SameKind, P...> == 4) constexpr auto _adl_em_any_of_elementwise(std::nullptr_t, F &&func, P &&... params) noexcept(noexcept(auto(std::invoke(func, vec_elem(0, EM_FWD(params))...)))) -> decltype(auto(std::invoke(func, vec_elem(0, EM_FWD(params))...))) {if (auto d = std::invoke(func, vec_elem(0, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, vec_elem(1, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, vec_elem(2, EM_FWD(params))...)) return d; if (auto d = std::invoke(func, vec_elem(3, EM_FWD(params))...)) return d; return {};}
}
