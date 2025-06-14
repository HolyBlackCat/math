#pragma once

#include "em/macros/utils/forward.h"
#include "em/math/scalar.h"
#include "em/meta/common.h"

#include <type_traits>

namespace em::Math
{
    namespace Customize
    {
        template <Meta::cvref_unqualified T>
        struct VectorTraits
        {
            static constexpr int size = 1;
            using type = T;

            // This function must not forward if it's going to return the same element for different indices, like we're doing here for scalars.
            [[nodiscard]] static constexpr auto &&GetElem(int i, Meta::same_or_derived_from_ignoring_cvref<T> auto &&v)
            {
                (void)i;
                return v; // Intentionally no forwarding.
            }
        };
    }

    // If `T` is a vector (maybe cvref-qualified), returns its base type, otherwise returns just that cvref-unqualified type.
    template <typename T>
    using vec_base_t = typename Customize::VectorTraits<std::remove_cvref_t<T>>::type;

    // If `T` is a vector (maybe cvref-qualified), returns its base type with cvref copied from T. Otherwise returns exactly `T`.
    template <typename T>
    using vec_base_cvref_t = Meta::copy_cvref<T, vec_base_t<T>>;

    // If `T` is a vector (maybe cvref-qualified), returns its size. Otherwise returns 1.
    // If multiple arguments are specified, all of them must have the exact same size, otherwise causes a SFINAE error.
    template <typename T, typename ...P> requires ((Customize::VectorTraits<std::remove_cvref_t<T>>::size == Customize::VectorTraits<std::remove_cvref_t<P>>::size) && ...)
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

    // Checks that all vectors in `P0, P...` have the same size, ignoring non-vectors.
    // Returns true if there are no vectors, or if the list empty.
    // Unlike the plain `vec_size`, this ignores scalars.
    template <typename ...P>
    concept have_common_vec_size = common_vec_size_or_zero<P...> != 0;

    // Returns the size of all vectors (maybe cvref). If the sizes don't match, triggers a SFINAE error.
    // If there are only scalars, return 1. If the list is empty, returns 1.
    // Unlike the plain `vec_size`, this ignores scalars.
    template <typename ...P> requires have_common_vec_size<P...>
    static constexpr int common_vec_size = common_vec_size_or_zero<P...>;

    // Some concepts:

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

    // Returns i-th element of a vector. If `v` is not a vector, returns it as is.
    // Prefer `apply_elementwise` to looping over `vec_elem`, because `apply_elementwise` supports more types (rects, etc).
    // No `vector_or_scalar_cvref` to allow arbitrary types.
    template <typename T>
    [[nodiscard]] constexpr auto &&vec_elem(int i, T &&v) noexcept
    {
        return Customize::VectorTraits<std::remove_cvref_t<T>>::GetElem(i, EM_FWD(v));
    }
}
