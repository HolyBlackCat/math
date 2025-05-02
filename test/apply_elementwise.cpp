#include "em/math/apply_elementwise.h"
#include "em/math/vector.h"

#include <type_traits>

// --- Macros:

namespace
{
    struct A
    {
        EM_APPLICABLE_ELEMENTWISE

        constexpr int operator()(int x, int y) {return x + y;}
    };
}

static_assert(A{}(10, 20) == 30);

static_assert(std::is_same_v<decltype(A{}(em::ivec3(1,2,3), 10)), em::ivec3>);
static_assert(A{}(em::ivec3(1,2,3), 10).x == 11);
static_assert(A{}(em::ivec3(1,2,3), 10).y == 12);
static_assert(A{}(em::ivec3(1,2,3), 10).z == 13);

static_assert(std::is_same_v<decltype(A{}(em::ivec3(1,2,3), em::ivec3(10,20,30))), em::ivec3>);
static_assert(A{}(em::ivec3(1,2,3), em::ivec3(10,20,30)).x == 11);
static_assert(A{}(em::ivec3(1,2,3), em::ivec3(10,20,30)).y == 22);
static_assert(A{}(em::ivec3(1,2,3), em::ivec3(10,20,30)).z == 33);

static_assert(std::is_same_v<decltype(A{}(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10)), em::vec2<em::ivec3>>);
static_assert(A{}(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.x == 11);
static_assert(A{}(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.y == 12);
static_assert(A{}(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).x.z == 13);
static_assert(A{}(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).y.x == 14);
static_assert(A{}(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).y.y == 15);
static_assert(A{}(em::vec2(em::ivec3(1,2,3), em::ivec3(4,5,6)), 10).y.z == 16);

template <typename T, typename U>
concept CanA = requires{A{}(std::declval<T>(), std::declval<U>());};

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
        EM_APPLICABLE_ELEMENTWISE_SAME_KIND

        constexpr int operator()(int x, int y) {return x + y;}
    };
}

static_assert(B{}(10, 20) == 30);

static_assert(std::is_same_v<decltype(B{}(em::ivec3(1,2,3), em::ivec3(10,20,30))), em::ivec3>);
static_assert(B{}(em::ivec3(1,2,3), em::ivec3(10,20,30)).x == 11);
static_assert(B{}(em::ivec3(1,2,3), em::ivec3(10,20,30)).y == 22);
static_assert(B{}(em::ivec3(1,2,3), em::ivec3(10,20,30)).z == 33);

template <typename T, typename U>
concept CanB = requires{B{}(std::declval<T>(), std::declval<U>());};

static_assert(CanB<int, int>);
static_assert(CanB<em::ivec3, em::fvec3>); // Different element types are allowed.
static_assert(CanB<em::vec2<em::ivec3>, em::vec2<em::fvec3>>); // Different element types are allowed.
static_assert(!CanB<em::ivec3, int>); // This isn't allowed.
static_assert(!CanB<em::vec2<em::ivec3>, int>); // This isn't allowed.
static_assert(!CanB<em::vec2<em::ivec3>, em::ivec2>); // Structure mismatch, intentionally not allowing this for now.
static_assert(!CanB<int, const char *>);
static_assert(!CanB<em::ivec3, const char *>);



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
