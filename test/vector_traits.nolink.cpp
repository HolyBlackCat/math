#include "em/math/vector.h"

// Using those to check that we're not limited to scalars.
struct A {};
struct B {};

static_assert(std::is_same_v<em::Math::change_vec_base<em::vec3<A>, A>, em::vec3<A>>); // Identity.
static_assert(std::is_same_v<em::Math::change_vec_base<em::vec3<A>, B>, em::vec3<B>>);

static_assert(std::is_same_v<em::Math::change_vec_size<em::vec3<A>, 3>, em::vec3<A>>); // Identity.
static_assert(std::is_same_v<em::Math::change_vec_size<em::vec3<A>, 4>, em::vec4<A>>);
static_assert(std::is_same_v<em::Math::change_vec_size<em::vec3<A>, 1>, A>); // Shrink a scalar.

template <typename V, typename T>
concept CanChangeBase = requires{typename em::Math::change_vec_base<V, T>;};

static_assert(CanChangeBase<em::vec3<A>, A>);
static_assert(CanChangeBase<em::vec3<A>, B>);
static_assert(!CanChangeBase<em::vec3<A>, int &>);

template <typename V, int N>
concept CanChangeSize = requires{typename em::Math::change_vec_size<V, N>;};

static_assert(CanChangeSize<em::vec3<A>, 3>);
static_assert(CanChangeSize<em::vec3<A>, 4>);
static_assert(CanChangeSize<em::vec3<A>, 1>);
static_assert(!CanChangeSize<em::vec3<A>, 5>);
static_assert(!CanChangeSize<em::vec3<A>, 0>);
static_assert(!CanChangeSize<em::vec3<A>, -1>);
