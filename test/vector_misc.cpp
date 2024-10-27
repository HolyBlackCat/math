#include "em/math/vector.h"

// Type shorthands.
static_assert(std::is_same_v<em::Math::Common::vec2<int>, em::Math::vec<int, 2>>);
static_assert(std::is_same_v<em::Math::Common::ivec<2>, em::Math::vec<int, 2>>);
static_assert(std::is_same_v<em::Math::Common::ivec2, em::Math::vec<int, 2>>);

// `larger_t`.
static_assert(std::is_same_v<em::Math::larger_t<em::Math::vec<int, 3>>, em::Math::vec<int, 3>>);
static_assert(std::is_same_v<em::Math::larger_t<em::Math::vec<int, 3>, float>, em::Math::vec<float, 3>>);
static_assert(std::is_same_v<em::Math::larger_t<em::Math::vec<float, 3>, int>, em::Math::vec<float, 3>>);
static_assert(std::is_same_v<em::Math::larger_t<float, em::Math::vec<int, 3>>, em::Math::vec<float, 3>>);
static_assert(std::is_same_v<em::Math::larger_t<em::Math::vec<int, 3>, em::Math::vec<float, 3>>, em::Math::vec<float, 3>>);

// `can_safely_convert`.
static_assert(em::Math::can_safely_convert<em::Math::vec<int, 3>, em::Math::vec<int, 3>>);
static_assert(!em::Math::can_safely_convert<em::Math::vec<int, 3>, em::Math::vec<int, 2>>);
static_assert(!em::Math::can_safely_convert<em::Math::vec<int, 2>, em::Math::vec<int, 3>>);

static_assert(em::Math::can_safely_convert<em::Math::vec<int, 3>, em::Math::vec<float, 3>>);
static_assert(!em::Math::can_safely_convert<em::Math::vec<float, 3>, em::Math::vec<int, 3>>);

static_assert(em::Math::can_safely_convert<int, em::Math::vec<float, 3>>);
static_assert(!em::Math::can_safely_convert<em::Math::vec<float, 3>, int>);

static_assert(!em::Math::can_safely_convert<float, em::Math::vec<int, 3>>);
static_assert(!em::Math::can_safely_convert<em::Math::vec<int, 3>, float>);

static_assert(em::Math::can_safely_convert<const em::Math::vec<int, 3>, em::Math::vec<int, 3> &&>); // Ignores cvref.

// Conversions between vector types.
static_assert(em::Math::fvec4(em::Math::ivec4(1,2,3,4)) == em::Math::fvec4(1,2,3,4));
static_assert(em::Math::ivec4(em::Math::fvec4(1.8f,2.1f,3,4)) == em::Math::ivec4(1,2,3,4));
static_assert(std::is_constructible_v<em::Math::fvec4, em::Math::ivec4>);
static_assert(std::is_constructible_v<em::Math::ivec4, em::Math::fvec4>);
static_assert(!std::is_convertible_v<em::Math::fvec4, em::Math::ivec4>);
static_assert(std::is_convertible_v<em::Math::ivec4, em::Math::fvec4>);


// `.to<T>()`

static_assert(em::Math::fvec4(1.8f,2.1f,3,4).to<int>() == em::Math::ivec4(1,2,3,4));
