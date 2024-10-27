#include "em/math/larger_type.h"

static_assert(em::Math::can_safely_convert<int, int>);
static_assert(em::Math::can_safely_convert<float, float>);
static_assert(em::Math::can_safely_convert<int, float>);
static_assert(!em::Math::can_safely_convert<float, int>);
