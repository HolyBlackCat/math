#include "em/math/functions.h"
#include "em/math/vector.h"

#include <limits>

static_assert(em::deg_to_rad(180) == em::f_pi);
static_assert(em::rad_to_deg(em::f_pi) == 180);
static_assert(em::rad_to_deg(em::fvec2(0, em::f_pi)) == em::fvec2(0, 180));

static_assert(em::sign(10) == 1);
static_assert(em::sign(-10) == -1);
static_assert(em::sign(0) == 0);
static_assert(em::sign(em::fvec3(10,-10,0)) == em::ivec3(1,-1,0));

static_assert(em::diffsign(20, 10) == 1);
static_assert(em::diffsign(10, 20) == -1);
static_assert(em::diffsign(10, 10) == 0);
static_assert(em::diffsign(em::fvec3(20,10,10), em::fvec3(10,20,10)) == em::ivec3(1,-1,0));

static_assert(em::clamp(15, 10, 20) == 15);
static_assert(em::clamp(5, 10, 20) == 10);
static_assert(em::clamp(25, 10, 20) == 20);
static_assert(em::clamp(em::ivec3(15, 5, 25), 10, 20) == em::ivec3(15, 10, 20));
static_assert(em::clamp(em::ivec3(15, 5, 25), em::fvec3(15.5f, 10.5f, 10.5f), 20) == em::fvec3(15.5f, 10.5f, 20));

static_assert(em::clamp(15, 20, 10) == 20); // If the bounds are inverted, return the lower bounds. This is completely arbitrary.

static_assert(em::clamp_low(15, 10) == 15);
static_assert(em::clamp_low(15, 20) == 20);
static_assert(em::clamp_high(15, 20) == 15);
static_assert(em::clamp_high(15, 10) == 10);

static_assert(em::clamp_low(std::numeric_limits<float>::quiet_NaN(), 10) == 10);
static_assert(em::clamp_high(std::numeric_limits<float>::quiet_NaN(), 20) == 20);
static_assert(em::clamp(std::numeric_limits<float>::quiet_NaN(), 10, 20) == 10); // Default to the lower bound, why not.

static_assert(std::is_invocable_v<decltype(em::clamp), int, int, int>);
static_assert(std::is_invocable_v<decltype(em::clamp), float, int, int>);
static_assert(std::is_invocable_v<decltype(em::clamp), int, float, int>);
static_assert(std::is_invocable_v<decltype(em::clamp_var), int &, int, int>);
static_assert(std::is_invocable_v<decltype(em::clamp_var), float &, int, int>);
static_assert(!std::is_invocable_v<decltype(em::clamp_var), int &, float, int>); // This one is not allowed, because the larger type doesn't match the type of the first argument.
static_assert(std::is_invocable_v<decltype(em::clamp_var), em::fvec3 &, int, int>);
static_assert(!std::is_invocable_v<decltype(em::clamp_var), em::ivec3 &, float, int>); // Not allowed for the same reason.

static_assert([]{int i = 15; em::clamp_var(i, 10, 20); return i;}() == 15);
static_assert([]{int i = 5; em::clamp_var(i, 10, 20); return i;}() == 10);
static_assert([]{int i = 20; em::clamp_var(i, 10, 20); return i;}() == 20);
static_assert([]{em::ivec3 i(15, 5, 25); em::clamp_var(i, 10, 20); return i;}() == em::ivec3(15, 10, 20));
static_assert([]{em::ivec3 i(15, 5, 25); em::clamp_var(i, 10, 20); return i;}() == em::ivec3(15, 10, 20));
static_assert([]{em::fvec3 i(15.5f, 5.5f, 25.5f); em::clamp_var(i, 10, 20); return i;}() == em::fvec3(15.5f, 10.f, 20.f));

static_assert(em::clamp(15, 20, 10) == 20); // If the bounds are inverted, return the lower bounds. This is completely arbitrary.
static_assert([]{int i = 15; em::clamp_var(i, 20, 10); return i;}() == 20); // Same.

static_assert(em::clamp_low(15, 10) == 15);
static_assert(em::clamp_low(15, 20) == 20);
static_assert(em::clamp_high(15, 20) == 15);
static_assert(em::clamp_high(15, 10) == 10);

static_assert([]{int i = 15; em::clamp_var_low(i, 10); return i;}() == 15);
static_assert([]{int i = 5; em::clamp_var_low(i, 10); return i;}() == 10);
static_assert([]{int i = 15; em::clamp_var_high(i, 20); return i;}() == 15);
static_assert([]{int i = 25; em::clamp_var_high(i, 20); return i;}() == 20);

static_assert(em::clamp_low(std::numeric_limits<float>::quiet_NaN(), 10) == 10);
static_assert(em::clamp_high(std::numeric_limits<float>::quiet_NaN(), 20) == 20);
static_assert(em::clamp(std::numeric_limits<float>::quiet_NaN(), 10, 20) == 10); // Default to the lower bound, why not.

static_assert([]{float i = std::numeric_limits<float>::quiet_NaN(); em::clamp_var_low(i, 10); return i;}() == 10);
static_assert([]{float i = std::numeric_limits<float>::quiet_NaN(); em::clamp_var_high(i, 20); return i;}() == 20);
static_assert([]{float i = std::numeric_limits<float>::quiet_NaN(); em::clamp_var(i, 10, 20); return i;}() == 10); // Default to the lower bound, why not.


// clamp_abs
static_assert(em::clamp_abs(5, 10) == 5);
static_assert(em::clamp_abs(-15, 10) == -10);
static_assert(em::clamp_abs(15, 10) == 10);
static_assert(em::clamp_abs(std::numeric_limits<float>::quiet_NaN(), 10) == -10); // Arbitrary, to match the normal `clamp()`.
static_assert([]{int i = 5; em::clamp_var_abs(i, 10); return i;}() == 5);
static_assert([]{int i = -15; em::clamp_var_abs(i, 10); return i;}() == -10);
static_assert([]{int i = 15; em::clamp_var_abs(i, 10); return i;}() == 10);
static_assert([]{float i = std::numeric_limits<float>::quiet_NaN(); em::clamp_var_abs(i, 10); return i;}() == -10.f); // Arbitrary, to match the normal `clamp()`.


// clamp{,_low,_high,_abs} with implicit bounds

static_assert(em::clamp_low(-2) == 0);
static_assert(em::clamp_low(-1) == 0);
static_assert(em::clamp_low( 0) == 0);
static_assert(em::clamp_low( 1) == 1);
static_assert(em::clamp_low( 2) == 2);

static_assert(em::clamp_high(-2) == -2);
static_assert(em::clamp_high(-1) == -1);
static_assert(em::clamp_high( 0) ==  0);
static_assert(em::clamp_high( 1) ==  1);
static_assert(em::clamp_high( 2) ==  1);

static_assert(em::clamp(-2) == 0);
static_assert(em::clamp(-1) == 0);
static_assert(em::clamp( 0) == 0);
static_assert(em::clamp( 1) == 1);
static_assert(em::clamp( 2) == 1);

static_assert(em::clamp_abs(-2) == -1);
static_assert(em::clamp_abs(-1) == -1);
static_assert(em::clamp_abs( 0) ==  0);
static_assert(em::clamp_abs( 1) ==  1);
static_assert(em::clamp_abs( 2) ==  1);

static_assert([]{int i = -2; em::clamp_var_low(i); return i == 0;}());
static_assert([]{int i = -1; em::clamp_var_low(i); return i == 0;}());
static_assert([]{int i =  0; em::clamp_var_low(i); return i == 0;}());
static_assert([]{int i =  1; em::clamp_var_low(i); return i == 1;}());
static_assert([]{int i =  2; em::clamp_var_low(i); return i == 2;}());

static_assert([]{int i = -2; em::clamp_var_high(i); return i == -2;}());
static_assert([]{int i = -1; em::clamp_var_high(i); return i == -1;}());
static_assert([]{int i =  0; em::clamp_var_high(i); return i ==  0;}());
static_assert([]{int i =  1; em::clamp_var_high(i); return i ==  1;}());
static_assert([]{int i =  2; em::clamp_var_high(i); return i ==  1;}());

static_assert([]{int i = -2; em::clamp_var(i); return i == 0;}());
static_assert([]{int i = -1; em::clamp_var(i); return i == 0;}());
static_assert([]{int i =  0; em::clamp_var(i); return i == 0;}());
static_assert([]{int i =  1; em::clamp_var(i); return i == 1;}());
static_assert([]{int i =  2; em::clamp_var(i); return i == 1;}());

static_assert([]{int i = -2; em::clamp_var_abs(i); return i == -1;}());
static_assert([]{int i = -1; em::clamp_var_abs(i); return i == -1;}());
static_assert([]{int i =  0; em::clamp_var_abs(i); return i ==  0;}());
static_assert([]{int i =  1; em::clamp_var_abs(i); return i ==  1;}());
static_assert([]{int i =  2; em::clamp_var_abs(i); return i ==  1;}());


// abs

static_assert(em::abs(2) == 2);
static_assert(em::abs(-2) == 2);
static_assert(em::abs(em::ivec2(10, -20)) == em::ivec2(10, 20));


// div_ex, mod_ex

static_assert(em::div_ex(-4, 2) == -2);
static_assert(em::div_ex(-3, 2) == -2);
static_assert(em::div_ex(-2, 2) == -1);
static_assert(em::div_ex(-1, 2) == -1);
static_assert(em::div_ex( 0, 2) ==  0);
static_assert(em::div_ex( 1, 2) ==  0);
static_assert(em::div_ex( 2, 2) ==  1);
static_assert(em::div_ex( 3, 2) ==  1);
static_assert(em::div_ex( 4, 2) ==  2);

static_assert(em::div_ex(-4, -2) ==  2);
static_assert(em::div_ex(-3, -2) ==  2);
static_assert(em::div_ex(-2, -2) ==  1);
static_assert(em::div_ex(-1, -2) ==  1);
static_assert(em::div_ex( 0, -2) ==  0);
static_assert(em::div_ex( 1, -2) ==  0);
static_assert(em::div_ex( 2, -2) == -1);
static_assert(em::div_ex( 3, -2) == -1);
static_assert(em::div_ex( 4, -2) == -2);

static_assert(em::mod_ex(-4, 3) == 2);
static_assert(em::mod_ex(-3, 3) == 0);
static_assert(em::mod_ex(-2, 3) == 1);
static_assert(em::mod_ex(-1, 3) == 2);
static_assert(em::mod_ex( 0, 3) == 0);
static_assert(em::mod_ex( 1, 3) == 1);
static_assert(em::mod_ex( 2, 3) == 2);
static_assert(em::mod_ex( 3, 3) == 0);
static_assert(em::mod_ex( 4, 3) == 1);

static_assert(em::mod_ex(-4, -3) == 2);
static_assert(em::mod_ex(-3, -3) == 0);
static_assert(em::mod_ex(-2, -3) == 1);
static_assert(em::mod_ex(-1, -3) == 2);
static_assert(em::mod_ex( 0, -3) == 0);
static_assert(em::mod_ex( 1, -3) == 1);
static_assert(em::mod_ex( 2, -3) == 2);
static_assert(em::mod_ex( 3, -3) == 0);
static_assert(em::mod_ex( 4, -3) == 1);


// div_maxabs

static_assert(em::div_maxabs(-4, 3) == -2);
static_assert(em::div_maxabs(-3, 3) == -1);
static_assert(em::div_maxabs(-2, 3) == -1);
static_assert(em::div_maxabs(-1, 3) == -1);
static_assert(em::div_maxabs( 0, 3) ==  0);
static_assert(em::div_maxabs( 1, 3) ==  1);
static_assert(em::div_maxabs( 2, 3) ==  1);
static_assert(em::div_maxabs( 3, 3) ==  1);
static_assert(em::div_maxabs( 4, 3) ==  2);
static_assert(em::div_maxabs(em::ivec2(5, 2), 3) == em::ivec2(2, 1));


// ipow

static_assert(em::ipow(3, 0) == 1);
static_assert(em::ipow(3, 1) == 3);
static_assert(em::ipow(3, 2) == 9);
static_assert(em::ipow(3, 3) == 27);
static_assert(em::ipow(3, -1) == 1); // For now negative powers are treated as zeroes.
static_assert(em::ipow(em::ivec2(2, 3), 2) == em::ivec2(4, 9));


// smoothstep

static_assert(em::smoothstep(0.f) == 0.f);
static_assert(em::smoothstep(1.f) == 1.f);


// mix

static_assert(em::mix(0, 10, 20) == 10);
static_assert(em::mix(1, 10, 20) == 20);


// sort_two[_var]

static_assert(em::sort_two(10, 20) == std::pair(10, 20));
static_assert(em::sort_two(20, 10) == std::pair(10, 20));
static_assert(em::sort_two(em::ivec2(20, 30), em::ivec2(10, 40)) == std::pair(em::ivec2(10, 30), em::ivec2(20, 40)));

static_assert([]{int a = 10, b = 20; em::sort_two_var(a, b); return a == 10 && b == 20;}());
static_assert([]{int a = 20, b = 10; em::sort_two_var(a, b); return a == 10 && b == 20;}());
static_assert([]{em::ivec2 a(20, 30), b(10, 40); em::sort_two_var(a, b); return a == em::ivec2(10, 30) && b == em::ivec2(20, 40);}());

// The non-`_var` version can handle different types:
static_assert(em::sort_two(1, 2.3f) == std::pair(1.f, 2.3f));
static_assert(em::sort_two(em::ivec2(20, 30), em::fvec2(10.1f, 40.4f)) == std::pair(em::fvec2(10.1f, 30.f), em::fvec2(20.f, 40.4f)));



// ----- Non-`Common` functions

static_assert(std::is_same_v<decltype(em::Math::next_value_towards(10, 20)), int>);
static_assert(em::Math::next_value_towards(10, 20) == 11);
static_assert(em::Math::next_value_towards(10, 0) == 9);
static_assert(em::Math::next_value_towards(em::ivec2(20, 10), 15) == em::ivec2(19, 11));

static_assert(std::is_same_v<decltype(em::Math::next_value_towards(10.f, 20)), float>);
static_assert(std::is_same_v<decltype(em::Math::next_value_towards(10.f, 20.f)), float>);
// Those are not constexpr sadly.
// static_assert(em::Math::next_value_towards(10, 20.f) == );
// static_assert(em::Math::next_value_towards(10.5f, 20) == );
// static_assert(em::Math::next_value_towards(10.5f, 20.5f) == );

static_assert(em::Math::next_value(10) == 11);
static_assert(em::Math::prev_value(10) == 9);
static_assert(em::Math::next_value(em::ivec2(10, 20)) == em::ivec2(11, 21));
