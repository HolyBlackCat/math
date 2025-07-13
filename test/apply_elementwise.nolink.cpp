#include "em/math/apply_elementwise.h"
#include "em/math/vector.h"

#include <type_traits>

// --- Macros:

namespace
{
    struct A
    {
        constexpr int operator()(int x, int y) const {return x + y;}
    };
    constexpr em::Math::ApplyElementwiseFn<A, {}> a{};
}

static_assert(a(10, 20) == 30);

static_assert(std::is_same_v<decltype(a(em::ivec3(1,2,3), 10)), em::ivec3>);
static_assert(a(em::ivec3(1,2,3), 10).x == 11);
static_assert(a(em::ivec3(1,2,3), 10).y == 12);
static_assert(a(em::ivec3(1,2,3), 10).z == 13);

static_assert(std::is_same_v<decltype(a(em::ivec3(1,2,3), em::ivec3(10,20,30))), em::ivec3>);
static_assert(a(em::ivec3(1,2,3), em::ivec3(10,20,30)).x == 11);
static_assert(a(em::ivec3(1,2,3), em::ivec3(10,20,30)).y == 22);
static_assert(a(em::ivec3(1,2,3), em::ivec3(10,20,30)).z == 33);

static_assert(std::is_same_v<decltype(a(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10)), em::vec2<em::ivec3>>);
static_assert(a(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.x == 11);
static_assert(a(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.y == 12);
static_assert(a(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.z == 13);
static_assert(a(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).y.x == 14);
static_assert(a(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).y.y == 15);
static_assert(a(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).y.z == 16);

template <typename T, typename U>
concept CanA = requires{a(std::declval<T>(), std::declval<U>());};

static_assert(CanA<int, int>);
static_assert(CanA<em::ivec3, int>);
static_assert(CanA<em::ivec3, em::fvec3>);
static_assert(CanA<em::vec3<em::ivec3>, int>);
static_assert(!CanA<int, const char *>);
static_assert(!CanA<em::ivec3, const char *>);


namespace
{
    struct B
    {
        constexpr int operator()(int x, int y) const {return x + y;}
    };
    constexpr em::Math::ApplyElementwiseFn<B, em::Math::ApplyElementwiseFlags::same_kind> b{};
}

static_assert(b(10, 20) == 30);

static_assert(std::is_same_v<decltype(b(em::ivec3(1,2,3), em::ivec3(10,20,30))), em::ivec3>);
static_assert(b(em::ivec3(1,2,3), em::ivec3(10,20,30)).x == 11);
static_assert(b(em::ivec3(1,2,3), em::ivec3(10,20,30)).y == 22);
static_assert(b(em::ivec3(1,2,3), em::ivec3(10,20,30)).z == 33);

template <typename T, typename U>
concept CanB = requires{b(std::declval<T>(), std::declval<U>());};

static_assert(CanB<int, int>);
static_assert(CanB<em::ivec3, em::fvec3>); // Different element types are allowed.
static_assert(CanB<em::vec2<em::ivec3>, em::vec2<em::fvec3>>); // Different element types are allowed.
static_assert(!CanB<em::ivec3, int>); // This isn't allowed.
static_assert(!CanB<em::vec2<em::ivec3>, int>); // This isn't allowed.
static_assert(!CanB<em::vec2<em::ivec3>, em::ivec2>); // Structure mismatch, intentionally not allowing this for now.
static_assert(!CanB<int, const char *>);
static_assert(!CanB<em::ivec3, const char *>);


// Make sure the direct application of the function is preferred:

namespace
{
    struct C
    {
        constexpr bool operator()(int, int) const {return false;}
        constexpr bool operator()(auto &&...) const {return true;}
    };
    constexpr em::Math::ApplyElementwiseFn<C, {}> c{};
}

static_assert(c(em::Math::ivec3(1,2,3), em::Math::ivec3(4,5,6)));


// --- Functions:

static_assert(em::Math::apply_elementwise([](int x, int y){return x + y;}, 10, 20) == 30);
static_assert(em::Math::apply_elementwise([](int x, int y){return x + y;}, em::ivec3(1,2,3), 10).y == 12);
static_assert(em::Math::apply_elementwise([](int x, int y){return x + y;}, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.y == 12);

static auto l1 = [](int x, int y){return x + y;};
static_assert(em::Math::apply_elementwise(l1, 10, 20) == 30);
static_assert(em::Math::apply_elementwise(l1, em::ivec3(1,2,3), 10).y == 12);
static_assert(em::Math::apply_elementwise(l1, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.y == 12);


// `any_of_elementwise`:

static_assert(em::Math::apply_elementwise([](int x, int y){return x + y;}, 10, 20) == 30);
static_assert(em::Math::apply_elementwise([](int x, int y){return x + y;}, em::ivec3(1,2,3), 10).y == 12);
static_assert(em::Math::apply_elementwise([](int x, int y){return x + y;}, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.y == 12);

static auto l2 = [](int x, int y){return x == y;};
static_assert(em::Math::any_of_elementwise(l2, 10, 20) == false);
static_assert(em::Math::any_of_elementwise(l2, 10, 10) == true);
static_assert(em::Math::any_of_elementwise(l2, em::ivec3(1,2,3), 10) == false);
static_assert(em::Math::any_of_elementwise(l2, em::ivec3(1,2,3), 2) == true);
static_assert(em::Math::any_of_elementwise(l2, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10) == false);
static_assert(em::Math::any_of_elementwise(l2, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 2) == true);
static_assert(em::Math::any_of_elementwise(l2, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 5) == true);

static_assert(std::is_same_v<bool, decltype(em::Math::any_of_elementwise(l2, 10, 20))>);
static_assert(std::is_same_v<bool, decltype(em::Math::any_of_elementwise(l2, 10, 10))>);
static_assert(std::is_same_v<bool, decltype(em::Math::any_of_elementwise(l2, em::ivec3(1,2,3), 10) == false)>);
static_assert(std::is_same_v<bool, decltype(em::Math::any_of_elementwise(l2, em::ivec3(1,2,3), 2) == true)>);
static_assert(std::is_same_v<bool, decltype(em::Math::any_of_elementwise(l2, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10) == false)>);
static_assert(std::is_same_v<bool, decltype(em::Math::any_of_elementwise(l2, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 2) == true)>);
static_assert(std::is_same_v<bool, decltype(em::Math::any_of_elementwise(l2, em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 5) == true)>);


// Make sure the `nontrivial` flag isn't broken:

static_assert(em::Math::any_of_elementwise<em::Math::ApplyElementwiseFlags::nontrivial>([](int,int){return true;}, em::ivec3(1,2,3), em::ivec3(1,2,2)));

namespace
{
    template <em::Math::ApplyElementwiseFlags Flags, typename ...P>
    concept CanAnyOfElementwise = requires{em::Math::any_of_elementwise<Flags>(std::declval<P>()...);};
}

[[maybe_unused]] static auto l3 = [](int, int){return true;};
static_assert( CanAnyOfElementwise<em::Math::ApplyElementwiseFlags::nontrivial, decltype(l3), em::ivec3, em::ivec3>);
static_assert( CanAnyOfElementwise<em::Math::ApplyElementwiseFlags::nontrivial, decltype(l3), em::ivec3, int>);
static_assert(!CanAnyOfElementwise<em::Math::ApplyElementwiseFlags::nontrivial, decltype(l3), int, int>);
static_assert( CanAnyOfElementwise<em::Math::ApplyElementwiseFlags::nontrivial | em::Math::ApplyElementwiseFlags::same_kind, decltype(l3), em::ivec3, em::ivec3>);
static_assert(!CanAnyOfElementwise<em::Math::ApplyElementwiseFlags::nontrivial | em::Math::ApplyElementwiseFlags::same_kind, decltype(l3), em::ivec3, int>);
static_assert(!CanAnyOfElementwise<em::Math::ApplyElementwiseFlags::nontrivial | em::Math::ApplyElementwiseFlags::same_kind, decltype(l3), int, int>);


// All of:
static_assert( em::Math::all_of_elementwise(l2, 10, 10));
static_assert(!em::Math::all_of_elementwise(l2, 10, 20));
static_assert( em::Math::all_of_elementwise(l2, em::ivec3(10, 20, 30), em::ivec3(10, 20, 30)));
static_assert(!em::Math::all_of_elementwise(l2, em::ivec3(10, 20, 30), em::ivec3(10, 20, 40)));
static_assert(!em::Math::all_of_elementwise(l2, em::ivec3(10, 20, 30), em::ivec3(10, 30, 40)));
static_assert(!em::Math::all_of_elementwise(l2, em::ivec3(10, 20, 30), em::ivec3(20, 30, 40)));

static_assert( em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::nontrivial>(l2, em::ivec3(10, 20, 30), em::ivec3(10, 20, 30)));
static_assert(!em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::nontrivial>(l2, em::ivec3(10, 20, 30), em::ivec3(10, 20, 40)));
static_assert(!em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::nontrivial>(l2, em::ivec3(10, 20, 30), em::ivec3(10, 30, 40)));
static_assert(!em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::nontrivial>(l2, em::ivec3(10, 20, 30), em::ivec3(20, 30, 40)));

static_assert( em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::same_kind>(l2, 10, 10));
static_assert(!em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::same_kind>(l2, 10, 20));
static_assert( em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::same_kind>(l2, em::ivec3(10, 20, 30), em::ivec3(10, 20, 30)));
static_assert(!em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::same_kind>(l2, em::ivec3(10, 20, 30), em::ivec3(10, 20, 40)));
static_assert(!em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::same_kind>(l2, em::ivec3(10, 20, 30), em::ivec3(10, 30, 40)));
static_assert(!em::Math::all_of_elementwise<em::Math::ApplyElementwiseFlags::same_kind>(l2, em::ivec3(10, 20, 30), em::ivec3(20, 30, 40)));

static_assert( em::Math::all_of_elementwise(l2, em::vec2(em::ivec2(10, 20), em::ivec2(30, 40)), em::vec2(em::ivec2(10, 20), em::ivec2(30, 40))));
static_assert(!em::Math::all_of_elementwise(l2, em::vec2(em::ivec2(10, 20), em::ivec2(30, 40)), em::vec2(em::ivec2(10, 20), em::ivec2(30, 50))));
static_assert(!em::Math::all_of_elementwise(l2, em::vec2(em::ivec2(10, 20), em::ivec2(30, 40)), em::vec2(em::ivec2(10, 30), em::ivec2(30, 40))));
static_assert(!em::Math::all_of_elementwise(l2, em::vec2(em::ivec2(10, 20), em::ivec2(20, 40)), em::vec2(em::ivec2(10, 30), em::ivec2(30, 40))));
