#pragma once

#include "em/macros/portable/tiny_func.h"
#include "em/macros/utils/returns.h"
#include "em/meta/common.h"

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

    // Integral scalars of a specific bit width.
    template <typename T, int NumBits> concept signed_scalar_cvref_bits = signed_integral_scalar_cvref<T> && sizeof(T) * 8 == NumBits;
    template <typename T, int NumBits> concept signed_scalar_bits = signed_integral_scalar<T> && sizeof(T) * 8 == NumBits;
    template <typename T, int NumBits> concept unsigned_scalar_cvref_bits = unsigned_integral_scalar_cvref<T> && sizeof(T) * 8 == NumBits;
    template <typename T, int NumBits> concept unsigned_scalar_bits = unsigned_integral_scalar<T> && sizeof(T) * 8 == NumBits;


    // What value to use when we prefer a `1`, but don't really care if it's not either.
    // Such as what to put as the `.w` when calling `.to_vec4()` on a vector.
    // By default this is `T(1)` if that compiles, otherwise `T{}`.
    template <scalar T>
    [[nodiscard]] EM_TINY constexpr auto OneIfScalar() EM_RETURNS(T(1))
    template <Meta::cvref_unqualified T>
    [[nodiscard]] EM_TINY constexpr auto OneIfScalar() EM_RETURNS(T{})
}
