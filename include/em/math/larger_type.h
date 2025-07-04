#pragma once

#include "em/math/scalar.h"
#include "em/meta/common.h"
#include "em/meta/reduce.h"

#include <compare>

namespace em::Math
{
    namespace detail
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
                // We could do something funny here with making `unsigned short` smaller than `int`,
                //   but that creates issues because `larger_t<...>` stops being order-independent with this.
                // Instead a proper fix would be to do this: if one type is unsigned and another is unsigned,
                //   the unsigned one is replaced with the next larger signed, and then the normal procedure follows.
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
            template <Meta::cvref_unqualified A, Meta::cvref_unqualified B> requires (detail::compare_types_v<A, B> == std::partial_ordering::equivalent) struct LargerType<A, B> {using type = A;};
            template <Meta::cvref_unqualified A, Meta::cvref_unqualified B> requires (detail::compare_types_v<A, B> == std::partial_ordering::less      ) struct LargerType<A, B> {using type = B;};
            template <Meta::cvref_unqualified A, Meta::cvref_unqualified B> requires (detail::compare_types_v<A, B> == std::partial_ordering::greater   ) struct LargerType<A, B> {using type = A;};
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
    concept can_safely_convert_to = Customize::CanSafelyConvert<std::remove_cvref_t<T>, std::remove_cvref_t<U>>::value;


    // Make a type floating-point.
    template <typename T>
    concept have_floating_point_type = have_larger_type<T, float>;

    template <Meta::cvref_unqualified T> requires have_floating_point_type<T>
    using floating_point_t = larger_t<T, float>;
}
