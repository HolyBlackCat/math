#pragma once

#include "em/meta/qualifiers.h"

#include <concepts>
#include <type_traits>

namespace em::Math
{
    // Note that our vectors are not limited to holding scalars.
    // Note that you can specialize the traits below to make certain classes count as scalars.

    namespace Customize
    {
        // If you customize those, it's your job to ensure at most one of them is true for every type.
        template <Meta::cvref_unqualified T> struct ScalarIsSignedIntegral : std::bool_constant<std::signed_integral<T>> {};
        template <Meta::cvref_unqualified T> struct ScalarIsUnsignedIntegral : std::bool_constant<std::unsigned_integral<T>> {};
        template <Meta::cvref_unqualified T> struct ScalarIsFloatingPoint : std::is_floating_point<T> {};
    }

    template <typename T> concept floating_point_scalar_cvref = Customize::ScalarIsFloatingPoint<std::remove_cvref_t<T>>::value;
    template <typename T> concept floating_point_scalar = Meta::cvref_unqualified<T> && floating_point_scalar_cvref<T>;

    template <typename T> concept signed_integral_scalar_cvref = Customize::ScalarIsSignedIntegral<std::remove_cvref_t<T>>::value;
    template <typename T> concept signed_integral_scalar = Meta::cvref_unqualified<T> && signed_integral_scalar_cvref<T>;

    template <typename T> concept unsigned_integral_scalar_cvref = Customize::ScalarIsUnsignedIntegral<std::remove_cvref_t<T>>::value;
    template <typename T> concept unsigned_integral_scalar = Meta::cvref_unqualified<T> && unsigned_integral_scalar_cvref<T>;

    template <typename T> concept integral_scalar_cvref = signed_integral_scalar_cvref<T> || unsigned_integral_scalar_cvref<T>;
    template <typename T> concept integral_scalar = signed_integral_scalar<T> || unsigned_integral_scalar<T>;

    template <typename T> concept scalar_cvref = integral_scalar_cvref<T> || floating_point_scalar_cvref<T>;
    template <typename T> concept scalar = integral_scalar<T> || floating_point_scalar<T>;
}
