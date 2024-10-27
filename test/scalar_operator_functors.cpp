#include "em/math/operator_functors.h"

#include <string>
#include <type_traits>

// Basics.
static_assert(em::Math::Ops::Neg{}(42) == -42);
static_assert(em::Math::Ops::Mul{}(10, 20) == 200);
static_assert(em::Math::Ops::Lshift{}(1, 2) == 4);

// Return types.
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}((char)1, (char)2)), char>);
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}((char)1, (short)2)), short>);
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}((short)1, (char)2)), short>);
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}((short)1, (long long)2)), long long>);
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}((float)1, (long long)2)), float>);
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}((double)1, (float)2)), double>);
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}((long double)1, (double)2)), long double>);
static_assert(std::is_same_v<decltype(em::Math::Ops::Add{}(std::string{}, std::string{})), std::string>);

// Noexcept-ness.
static_assert(noexcept(em::Math::Ops::Add{}(1, 2)));
static_assert(!noexcept(em::Math::Ops::Add{}(std::string{}, std::string{})));

// Negative tests.
template <typename A, typename B>
concept CanAdd = requires{em::Math::Ops::Add{}(std::declval<A>(), std::declval<B>());};
static_assert(!CanAdd<int, unsigned int>);
static_assert(!CanAdd<short, unsigned int>);
static_assert(!CanAdd<int, unsigned short>);
static_assert(!CanAdd<int, std::string>);

// Compound assignment.
static_assert([]{
    int x = 10;
    auto &&ret = em::Math::Ops::AddAssign{}(x, 42);
    static_assert(std::is_same_v<decltype(ret), int &>);
    if (&ret != &x)
        throw "Wrong address?";
    return x == 52;
}());

// Compound assignment combinations.
template <typename A, typename B>
concept CanAddAssign = requires{em::Math::Ops::AddAssign{}(std::declval<A>(), std::declval<B>());};
static_assert(CanAddAssign<int &, int>);
static_assert(!CanAddAssign<int, int>); // Builtin scalars reject rvalue assignment.
static_assert(CanAddAssign<std::string &, std::string>);
static_assert(!CanAddAssign<std::string, std::string>); // Force-reject rvalue lhs everywhere, even if the underlying type supports it.
// Ban mixed signedness:
static_assert(!CanAddAssign<short &, unsigned int>);
static_assert(!CanAddAssign<int &, unsigned short>);
static_assert(!CanAddAssign<int &, std::string>);
// Mixed precision:
static_assert(CanAddAssign<int &, short>); // Increases precision.
static_assert(!CanAddAssign<short &, int>); // Reduces precision = disabled.



// Bitshifts:
template <typename A, typename B>
concept CanLshift = requires{em::Math::Ops::Lshift{}(std::declval<A>(), std::declval<B>());};
template <typename A, typename B>
concept CanLshiftAssign = requires{em::Math::Ops::LshiftAssign{}(std::declval<A>(), std::declval<B>());};

static_assert(CanLshift<int, int>);
static_assert(!CanLshift<std::string, std::string>);
static_assert(CanLshift<int, unsigned int>); // Mixed sign ok.
static_assert(CanLshift<unsigned short, int>); // ^

static_assert(std::is_same_v<decltype(em::Math::Ops::Lshift{}((char)1, (short)2)), char>); // Lhs type is preserved.
static_assert(std::is_same_v<decltype(em::Math::Ops::Lshift{}((short)1, (char)2)), short>); // ^

static_assert(CanLshiftAssign<unsigned short &, int>); // All assignment combinations ok, because only the lhs type matters.
static_assert(CanLshiftAssign<unsigned int &, short>); // ^




// Some special cases:

// Silently overflows, no UB because we internally promote to `unsigned int`.
static_assert(em::Math::Ops::Mul{}((unsigned short)0xffff, (unsigned short)0xffff) == (unsigned short)1);
