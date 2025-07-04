#include "em/math/larger_type.h"

static_assert(em::Math::can_safely_convert_to<int, int>);
static_assert(em::Math::can_safely_convert_to<float, float>);
static_assert(em::Math::can_safely_convert_to<int, float>);
static_assert(!em::Math::can_safely_convert_to<float, int>);
