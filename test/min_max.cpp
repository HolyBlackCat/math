#include "em/math/min_max.h"
#include "em/math/vector.h"

#include <string>

namespace
{
    struct A {};
}

// One argument.
static_assert(std::is_same_v<decltype(em::min(42)), int>);
static_assert(std::is_same_v<decltype(em::min(std::declval<int &>())), int>);
static_assert(em::min(42) == 42);
// One argument of a non-comparable type.
static_assert(std::is_same_v<decltype(em::min(A{})), A>);
static_assert(std::is_same_v<decltype(em::min(std::declval<A &>())), A>);

// Two+ arguments.
static_assert(em::min(10, 20) == 10);
static_assert(em::max(10, 20) == 20);
static_assert(em::min(10, 30, 20) == 10);
static_assert(em::min(30, 10, 20) == 10);
static_assert(em::min(30, 20, 10) == 10);
static_assert(em::min(30, 20, 40, 10) == 10);

// Strings?
static_assert(em::min(std::string("b"), std::string("a")) == "a");


// Vectors.
static_assert(em::min(em::ivec3(1,2,3), 2) == em::ivec3(1,2,2));
static_assert(em::max(em::ivec3(1,2,3), 2) == em::ivec3(2,2,3));

static_assert(std::is_same_v<decltype(em::min(em::ivec3(1,2,3), 2)), em::ivec3>);
static_assert(std::is_same_v<decltype(em::min(em::ivec3(1,2,3), 2.f)), em::fvec3>);
static_assert(std::is_same_v<decltype(em::min(em::fvec3(1,2,3), 2)), em::fvec3>);

namespace
{
    template <typename ...P> concept CanEmMin = requires{em::min(std::declval<P>()...);};
}

static_assert(!CanEmMin<em::ivec3, em::ivec2>);
static_assert(!CanEmMin<em::ivec3, unsigned>);

// Small integers?
static_assert(!CanEmMin<em::ivec3, unsigned long long>);
static_assert(!CanEmMin<em::ivec3, unsigned int>);
// This works, because the conversion is lossless.
static_assert(std::is_same_v<decltype(em::min(em::ivec3(1,2,3), (unsigned short)2)), em::ivec3>);
