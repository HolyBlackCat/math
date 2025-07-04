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
