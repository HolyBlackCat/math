#pragma once

#include "em/math/concepts_scalar.h"
#include "em/meta/cvref.h"
#include "em/meta/reduce.h"

#include <compare>

namespace em::Math
{
    namespace Customize
    {
        // Which type is "larger" than another.
        template <Meta::cvref_unqualified A, Meta::cvref_unqualified B>
        constexpr std::partial_ordering compare_types_v = []
        {
            if constexpr (std::is_same_v<A, B>)
                return std::partial_ordering::equivalent;
            else if (scalar<A> && scalar<B>)
            {
                if constexpr (floating_point_scalar<A> < floating_point_scalar<B>)
                    return std::partial_ordering::less;
                else if constexpr (floating_point_scalar<A> > floating_point_scalar<B>)
                    return std::partial_ordering::greater;
                else if constexpr (signed_integral_scalar<A> != signed_integral_scalar<B>)
                    return std::partial_ordering::unordered;
                else if constexpr (sizeof(A) < sizeof(B))
                    return std::partial_ordering::less;
                else if constexpr (sizeof(A) > sizeof(B))
                    return std::partial_ordering::greater;
                else
                    return std::partial_ordering::unordered;
            }
            else
                return std::partial_ordering::unordered;
        }();
    }

    namespace Customize
    {
        namespace Default
        {
            // The default implementation for the `LargerType` below. This is based on `Customize::compare_types_v`.
            template <Meta::cvref_unqualified A, Meta::cvref_unqualified B> struct LargerType {};
            template <Meta::cvref_unqualified A, Meta::cvref_unqualified B> requires (compare_types_v<A, B> == std::partial_ordering::equivalent) struct LargerType<A, B> {using type = A;};
            template <Meta::cvref_unqualified A, Meta::cvref_unqualified B> requires (compare_types_v<A, B> == std::partial_ordering::less      ) struct LargerType<A, B> {using type = B;};
            template <Meta::cvref_unqualified A, Meta::cvref_unqualified B> requires (compare_types_v<A, B> == std::partial_ordering::greater   ) struct LargerType<A, B> {using type = A;};
        }

        // Select a larger type from the two. For vectors this can return a completely different type (float + ivec2 = fvec2).
        // The default implementation picks larger of the two types based on `Customize::compare_types_v`.
        template <Meta::cvref_unqualified A, Meta::cvref_unqualified B>
        struct LargerType : Default::LargerType<A, B> {};
    }

    // Causes a soft error if there's no larger type.
    template <Meta::cvref_unqualified ...P>
    using larger_t = Meta::reduce_types_indirect<Customize::LargerType, P...>;

    template <typename ...P>
    concept have_larger_type = requires{typename larger_t<P...>;};

    namespace Customize
    {
        // Allow implicit conversions from `T` to `U`.
        template <Meta::cvref_unqualified T, Meta::cvref_unqualified U>
        struct CanSafelyConvert : std::false_type {};

        template <Meta::cvref_unqualified T, Meta::cvref_unqualified U> requires have_larger_type<T, U>
        struct CanSafelyConvert<T, U> : std::is_same<larger_t<T, U>, U> {};
    }

    // `U` is larger or equal to `T`. Ignores cvref-qualifiers.
    template <typename T, typename U>
    concept can_safely_convert = Customize::CanSafelyConvert<std::remove_cvref_t<T>, std::remove_cvref_t<U>>::value;
}
