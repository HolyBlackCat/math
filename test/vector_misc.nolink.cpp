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

// `can_safely_convert_to`.
static_assert(em::Math::can_safely_convert_to<em::Math::vec<int, 3>, em::Math::vec<int, 3>>);
static_assert(!em::Math::can_safely_convert_to<em::Math::vec<int, 3>, em::Math::vec<int, 2>>);
static_assert(!em::Math::can_safely_convert_to<em::Math::vec<int, 2>, em::Math::vec<int, 3>>);

static_assert(em::Math::can_safely_convert_to<em::Math::vec<int, 3>, em::Math::vec<float, 3>>);
static_assert(!em::Math::can_safely_convert_to<em::Math::vec<float, 3>, em::Math::vec<int, 3>>);

static_assert(em::Math::can_safely_convert_to<int, em::Math::vec<float, 3>>);
static_assert(!em::Math::can_safely_convert_to<em::Math::vec<float, 3>, int>);

static_assert(!em::Math::can_safely_convert_to<float, em::Math::vec<int, 3>>);
static_assert(!em::Math::can_safely_convert_to<em::Math::vec<int, 3>, float>);

static_assert(em::Math::can_safely_convert_to<const em::Math::vec<int, 3>, em::Math::vec<int, 3> &&>); // Ignores cvref.

// Conversions between vector types.
static_assert(em::Math::fvec4(em::Math::ivec4(1,2,3,4)) == em::Math::fvec4(1,2,3,4));
static_assert(em::Math::ivec4(em::Math::fvec4(1.8f,2.1f,3,4)) == em::Math::ivec4(1,2,3,4));
static_assert(std::is_constructible_v<em::Math::fvec4, em::Math::ivec4>);
static_assert(std::is_constructible_v<em::Math::ivec4, em::Math::fvec4>);
static_assert(!std::is_convertible_v<em::Math::fvec4, em::Math::ivec4>);
static_assert(std::is_convertible_v<em::Math::ivec4, em::Math::fvec4>);


// `.to<T>()`

static_assert(em::Math::fvec4(1.8f,2.1f,3,4).to<int>() == em::Math::ivec4(1,2,3,4));


// `.to_vecN()`

static_assert(em::ivec2(10,20).to_vec3() == em::ivec3(10,20,0));
static_assert(em::ivec2(10,20).to_vec4() == em::ivec4(10,20,0,1));
static_assert(em::ivec2(10,20).to_vec3(30) == em::ivec3(10,20,30));
static_assert(em::ivec2(10,20).to_vec4(30,40) == em::ivec4(10,20,30,40));

static_assert(em::ivec3(10,20,30).to_vec2() == em::ivec2(10,20));
static_assert(em::ivec3(10,20,30).to_vec4() == em::ivec4(10,20,30,1));
static_assert(em::ivec3(10,20,30).to_vec4(40) == em::ivec4(10,20,30,40));

static_assert(em::ivec4(10,20,30,40).to_vec2() == em::ivec2(10,20));
static_assert(em::ivec4(10,20,30,40).to_vec3() == em::ivec3(10,20,30));

template <typename T> concept CanCallToVec2 = requires(T t){t.to_vec2();};
template <typename T> concept CanCallToVec3 = requires(T t){t.to_vec3();};
template <typename T> concept CanCallToVec4 = requires(T t){t.to_vec4();};
static_assert(!CanCallToVec2<em::ivec2> && CanCallToVec2<em::ivec3> && CanCallToVec2<em::ivec4>);
static_assert(CanCallToVec3<em::ivec2> && !CanCallToVec3<em::ivec3> && CanCallToVec3<em::ivec4>);
static_assert(CanCallToVec4<em::ivec2> && CanCallToVec4<em::ivec3> && !CanCallToVec4<em::ivec4>);
