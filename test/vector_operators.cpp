#include "em/math/vector.h"

static_assert((-em::Math::vec<int, 2>(10, 20)).x == -10);
static_assert((-em::Math::vec<int, 2>(10, 20)).y == -20);
static_assert(std::is_same_v<decltype(-em::Math::vec<int, 2>()), em::Math::vec<int, 2>>);
static_assert(std::is_same_v<decltype(-em::Math::vec<short, 2>()), em::Math::vec<short, 2>>);
static_assert(noexcept(-em::Math::vec<int, 2>(10, 20)));

static_assert((em::Math::vec<int, 2>(10, 20) + em::Math::vec<int, 2>(100, 200)).x == 110);
static_assert((em::Math::vec<int, 2>(10, 20) + em::Math::vec<int, 2>(100, 200)).y == 220);

static_assert(std::is_same_v<decltype(em::Math::vec<short, 2>() - em::Math::vec<short, 2>()), em::Math::vec<short, 2>>);
static_assert(std::is_same_v<decltype(em::Math::vec<short, 2>() - em::Math::vec<int, 2>()), em::Math::vec<int, 2>>);
static_assert(std::is_same_v<decltype(em::Math::vec<int, 2>() - em::Math::vec<short, 2>()), em::Math::vec<int, 2>>);

// Comparisons:

static_assert(em::Math::vec<int, 2>(1, 2) == em::Math::vec<int, 2>(1, 2));
static_assert(em::Math::vec<int, 2>(1, 2) != em::Math::vec<int, 2>(1, 3));
static_assert(em::Math::vec<int, 2>(1, 2) != em::Math::vec<int, 2>(2, 3));

static_assert(em::Math::VectorOps::comparee_three_way(em::Math::vec<int, 2>(1, 2), em::Math::vec<int, 2>(1, 2)) == 0);
static_assert(em::Math::VectorOps::comparee_three_way(em::Math::vec<int, 2>(1, 2), em::Math::vec<int, 2>(1, 3)) < 0);
static_assert(em::Math::VectorOps::comparee_three_way(em::Math::vec<int, 2>(1, 2), em::Math::vec<int, 2>(2, 1)) < 0);
static_assert(em::Math::VectorOps::comparee_three_way(em::Math::vec<int, 2>(2, 1), em::Math::vec<int, 2>(1, 2)) > 0);

template <typename T, typename U>
concept EqComparable = requires{std::declval<T>() == std::declval<U>();};

static_assert(EqComparable<em::Math::vec<int, 4>, em::Math::vec<int, 4>>);
static_assert(!EqComparable<em::Math::vec<int, 4>, int>);
