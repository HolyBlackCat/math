#pragma once

#include "em/meta/packs.h"
#include "em/meta/qualifiers.h"

namespace em::Math
{
    namespace Customize
    {
        // Change the base type of a vector or another type. Defaults to just using the destination type.
        template <Meta::cvref_unqualified From, Meta::cvref_unqualified To>
        struct Rebind {using type = To;};

        // The default implementation for any template of the form `T<Type, Size, ExtraTypes...>`.
        template <template <typename, int, typename...> typename C, typename T, int N, typename ...P, typename U>
        struct Rebind<C<T, N, P...>, U> {using type = C<U, N, P...>;};

        // The default implementation for any template of the form `T<Type, ExtraTypes...>`.
        template <template <typename, typename...> typename C, typename T, typename ...P, typename U>
        struct Rebind<C<T, P...>, U> {using type = C<U, P...>;};
    }

    // Change the base type of vectors `P...` to `T`.
    // Returns the resulting type if it's the same for every vector, otherise fails.
    template <typename T, typename ...P>
    using rebind_to = Meta::require_same_type<typename Customize::Rebind<P, T>::type...>;
}
