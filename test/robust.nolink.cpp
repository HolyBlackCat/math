#include "em/math/robust.h"
#include "em/math/vector.h"

// Sanity check of the basic functions:

static_assert( em::Math::Robust::equal        (10, 10));
static_assert(!em::Math::Robust::not_equal    (10, 10));
static_assert(!em::Math::Robust::less         (10, 10));
static_assert(!em::Math::Robust::greater      (10, 10));
static_assert( em::Math::Robust::less_equal   (10, 10));
static_assert( em::Math::Robust::greater_equal(10, 10));
static_assert(em::Math::Robust::compare_three_way(10, 10) == std::strong_ordering::equivalent);

static_assert(!em::Math::Robust::equal        (10, 20));
static_assert( em::Math::Robust::not_equal    (10, 20));
static_assert( em::Math::Robust::less         (10, 20));
static_assert(!em::Math::Robust::greater      (10, 20));
static_assert( em::Math::Robust::less_equal   (10, 20));
static_assert(!em::Math::Robust::greater_equal(10, 20));
static_assert(em::Math::Robust::compare_three_way(10, 20) == std::strong_ordering::less);

static_assert(!em::Math::Robust::equal        (20, 10));
static_assert( em::Math::Robust::not_equal    (20, 10));
static_assert(!em::Math::Robust::less         (20, 10));
static_assert( em::Math::Robust::greater      (20, 10));
static_assert(!em::Math::Robust::less_equal   (20, 10));
static_assert( em::Math::Robust::greater_equal(20, 10));
static_assert(em::Math::Robust::compare_three_way(20, 10) == std::strong_ordering::greater);

static_assert(em::Math::Robust::compare(10) < 20);
static_assert(20 > em::Math::Robust::compare(10));
static_assert(em::Math::Robust::compare(10) == 10);
static_assert(10 == em::Math::Robust::compare(10));
static_assert(em::Math::Robust::compare(10) != 20);
static_assert(20 != em::Math::Robust::compare(10));

// Floating-point:

static_assert(em::Math::Robust::equal(12.3, 12.3));
static_assert(em::Math::Robust::less(12.3, 12.4));

// Mixed signedness:

static_assert(em::Math::Robust::less(-1, 10u));
static_assert(em::Math::Robust::greater(10u, -1));

static_assert(em::Math::Robust::less(-1, 2u));
static_assert(em::Math::Robust::less(-1, unsigned(0xfffffff0)));
static_assert(em::Math::Robust::less(1, 2u));
static_assert(em::Math::Robust::equal(2, 2u));

// Mixed floating-point-ness:

static_assert(em::Math::Robust::less(1, 2.f));
static_assert(em::Math::Robust::greater(2.f, 1));

static_assert(em::Math::Robust::less(1, 1.75f));
static_assert(em::Math::Robust::less(1, 1.25f));
static_assert(em::Math::Robust::greater(1, 0.75f));
static_assert(em::Math::Robust::greater(1, 0.25f));

static_assert(em::Math::Robust::greater(999999984306749440, 999999846867795968.f));
static_assert(em::Math::Robust::greater(999999984306749440, 999999915587272704.f));
static_assert(em::Math::Robust::equal(999999984306749440, 999999984306749440.f));
static_assert(em::Math::Robust::less(999999984306749440, 1000000053026226176.f));
static_assert(em::Math::Robust::less(999999984306749440, 1000000121745702912.f));

static_assert(em::Math::Robust::greater(999999984306749439, 999999846867795968.f));
static_assert(em::Math::Robust::greater(999999984306749439, 999999915587272704.f));
static_assert(em::Math::Robust::less(999999984306749439, 999999984306749440.f));
static_assert(em::Math::Robust::less(999999984306749439, 1000000053026226176.f));
static_assert(em::Math::Robust::less(999999984306749439, 1000000121745702912.f));


static_assert(em::Math::Robust::greater(100, 99.9999847412109375f));
static_assert(em::Math::Robust::greater(100, 99.99999237060546875f));
static_assert(em::Math::Robust::equal(100, 100.f));
static_assert(em::Math::Robust::less(100, 100.00000762939453125f));
static_assert(em::Math::Robust::less(100, 100.0000152587890625f));

static_assert(em::Math::Robust::greater(-100, -100.0000152587890625f));
static_assert(em::Math::Robust::greater(-100, -100.00000762939453125f));
static_assert(em::Math::Robust::equal(-100, -100.f));
static_assert(em::Math::Robust::less(-100, -99.99999237060546875f));
static_assert(em::Math::Robust::less(-100, -99.9999847412109375f));

static_assert(em::Math::Robust::greater(0, -2.8025969286496341418474591665798322625605238837530315435141365677795821653717212029732763767242431640625e-45f));
static_assert(em::Math::Robust::greater(0, -1.40129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125e-45f));
static_assert(em::Math::Robust::equal(0, -0.f));
static_assert(em::Math::Robust::equal(0, 0.f));
static_assert(em::Math::Robust::less(0, 1.40129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125e-45f));
static_assert(em::Math::Robust::less(0, 2.8025969286496341418474591665798322625605238837530315435141365677795821653717212029732763767242431640625e-45f));

static_assert(em::Math::Robust::greater((long long)0x8000'0000'0000'0000, -9223374235878031360.f));
static_assert(em::Math::Robust::greater((long long)0x8000'0000'0000'0000, -9223373136366403584.f));
static_assert(em::Math::Robust::equal((long long)0x8000'0000'0000'0000, -9223372036854775808.f));
static_assert(em::Math::Robust::less((long long)0x8000'0000'0000'0000, -9223371487098961920.f));
static_assert(em::Math::Robust::less((long long)0x8000'0000'0000'0000, -9223370937343148032.f));

static_assert(em::Math::Robust::less( 42, std::numeric_limits<float>::infinity()));
static_assert(em::Math::Robust::less(-42, std::numeric_limits<float>::infinity()));
static_assert(em::Math::Robust::greater( 42, -std::numeric_limits<float>::infinity()));
static_assert(em::Math::Robust::greater(-42, -std::numeric_limits<float>::infinity()));
static_assert(em::Math::Robust::compare_three_way( 42,  std::numeric_limits<float>::quiet_NaN()) == std::partial_ordering::unordered);
static_assert(em::Math::Robust::compare_three_way(-42,  std::numeric_limits<float>::quiet_NaN()) == std::partial_ordering::unordered);
static_assert(em::Math::Robust::compare_three_way( 42, -std::numeric_limits<float>::quiet_NaN()) == std::partial_ordering::unordered);
static_assert(em::Math::Robust::compare_three_way(-42, -std::numeric_limits<float>::quiet_NaN()) == std::partial_ordering::unordered);

static_assert(em::Math::Robust::greater(1388608, 1388607.75f));
static_assert(em::Math::Robust::greater(1388608, 1388607.875f));
static_assert(em::Math::Robust::equal(1388608, 1388608.f));
static_assert(em::Math::Robust::less(1388608, 1388608.125f));
static_assert(em::Math::Robust::less(1388608, 1388608.25f));

static_assert(em::Math::Robust::greater(12388608, 12388606.f));
static_assert(em::Math::Robust::greater(12388608, 12388607.f));
static_assert(em::Math::Robust::equal(12388608, 12388608.f));
static_assert(em::Math::Robust::less(12388608, 12388609.f));
static_assert(em::Math::Robust::less(12388608, 12388610.f));

// With vectors:
static_assert(em::Math::Robust::equal(em::ivec2(1, 2), em::fvec2(1, 2)));
static_assert(!em::Math::Robust::equal(em::ivec2(1, 2), em::fvec2(1, 3)));
static_assert(!em::Math::Robust::equal(em::ivec2(1, 2), em::fvec2(2, 3)));
static_assert(em::Math::Robust::equal(em::ivec2(1, 1), 1.f));
static_assert(!em::Math::Robust::equal(em::ivec2(1, 2), 1.f));
static_assert(!em::Math::Robust::equal(em::ivec2(2, 2), 1.f));


// "Representable as":

static_assert(em::Math::Robust::representable_as<float>(10));
// static_assert(!em::Math::Robust::representable_as<float>(0xffffffff)); // This is a hard error in constexpr. This is ok.
static_assert(em::Math::Robust::representable_as<float>(0x100000000));
static_assert(em::Math::Robust::representable_as<float>(0x100000000p0));

static_assert(em::Math::Robust::representable_as<char>(127));
static_assert(!em::Math::Robust::representable_as<char>(128));
static_assert(em::Math::Robust::representable_as<char>(-128));
static_assert(!em::Math::Robust::representable_as<char>(-129));

static_assert(!em::Math::Robust::representable_as<char>(126.3));
static_assert(em::Math::Robust::representable_as<char>(127.0));
// static_assert(!em::Math::Robust::representable_as<char>(128.0)); // Hard error, whatever.

static_assert(em::Math::Robust::representable_as<float>(1.5));
static_assert(!em::Math::Robust::representable_as<float>(1.3)); // An infinite fraction!
static_assert(em::Math::Robust::representable_as<double>(1.3f)); // Works in this direction

// With vectors:
static_assert(em::Math::Robust::representable_as<em::ivec2>(em::fvec2(1, 2)));
static_assert(!em::Math::Robust::representable_as<em::ivec2>(em::fvec2(1, 2.1f)));
static_assert(!em::Math::Robust::representable_as<em::ivec2>(em::fvec2(1.2f, 2.1f)));

static_assert(em::Math::Robust::representable_as<em::ivec2>(1.f));
static_assert(!em::Math::Robust::representable_as<em::ivec2>(1.2f));
