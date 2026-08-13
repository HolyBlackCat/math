#pragma once

#include "em/math/scalar.h"
#include "em/meta/common.h"
#include "em/meta/scalar_types.h"

#include <type_traits>

namespace em::Math
{
    namespace Customize
    {
        // Specializations of this should always check `CustomLargerType<P...>` to avoid ambiguities.
        template <typename ...P>
        struct LargerType {};

        template <typename ...P> requires Meta::Scalars::HaveLargerType<false, P...>
        struct LargerType<P...> {using type = Meta::Scalars::LargerType<false, P...>;};

        template <typename ...P>
        concept CustomLargerType = sizeof...(P) > 1 && !Meta::Scalars::HaveLargerType<false, P...>;
    }

    // Causes a soft error if there's no larger type.
    template <Meta::cvref_unqualified ...P> requires requires{typename Customize::LargerType<P...>::type;}
    using larger_t = typename Customize::LargerType<P...>::type;

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
