#include "em/math/vector.h"

// Using those to check that we're not limited to scalars.
struct A {friend bool operator==(A, A) = default;};
struct B {};

static_assert(std::is_same_v<em::Math::change_vec_base<A, em::vec3<A>>, em::vec3<A>>); // Identity.
static_assert(std::is_same_v<em::Math::change_vec_base<B, em::vec3<A>>, em::vec3<B>>);

static_assert(std::is_same_v<em::Math::change_vec_size<em::vec3<A>, 3>, em::vec3<A>>); // Identity.
static_assert(std::is_same_v<em::Math::change_vec_size<em::vec3<A>, 4>, em::vec4<A>>);
static_assert(std::is_same_v<em::Math::change_vec_size<em::vec3<A>, 1>, A>); // Shrink a scalar.

template <typename V, typename T>
concept CanChangeBase = requires{typename em::Math::change_vec_base<V, T>;};

static_assert(CanChangeBase<A, em::vec3<A>>);
static_assert(CanChangeBase<B, em::vec3<A>>);
static_assert(!CanChangeBase<em::vec3<A>, int &>);

template <typename V, int N>
concept CanChangeSize = requires{typename em::Math::change_vec_size<V, N>;};

static_assert(CanChangeSize<em::vec3<A>, 3>);
static_assert(CanChangeSize<em::vec3<A>, 4>);
static_assert(CanChangeSize<em::vec3<A>, 1>);
static_assert(!CanChangeSize<em::vec3<A>, 5>);
static_assert(!CanChangeSize<em::vec3<A>, 0>);
static_assert(!CanChangeSize<em::vec3<A>, -1>);


static_assert(em::Math::vec_reduce(42, []{}) == 42);
static_assert(std::is_same_v<decltype(em::Math::vec_reduce(42, []{})), int &&>);
static_assert(em::Math::vec_reduce(A{}, []{}) == A{});
static_assert(em::Math::vec_reduce(em::ivec2(1, 2), std::plus{}) == 3);
static_assert(em::Math::vec_reduce(em::ivec3(1, 2, 3), std::plus{}) == 6);
static_assert(em::Math::vec_reduce(em::ivec4(1, 2, 3, 4), std::plus{}) == 10);


static_assert(em::Math::vec_make_of_same_size<em::ivec2>(1.f, 2.f) == em::fvec2(1.f, 2.f));
static_assert(std::is_same_v<decltype(em::Math::vec_make_of_same_size<em::ivec2>(1.f, 2.f)), em::fvec2>);

static_assert(em::Math::vec_make_of_same_size<em::ivec2, char, em::uvec2>(1.f, 2.f) == em::fvec2(1.f, 2.f));
static_assert(std::is_same_v<decltype(em::Math::vec_make_of_same_size<em::ivec2, char, em::uvec2>(1.f, 2.f)), em::fvec2>);
