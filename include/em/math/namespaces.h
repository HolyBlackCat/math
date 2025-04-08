#pragma once

// This sets up our namespace hierarchy.

// Put popular stuff into `namespace Math::inline Common {...}` (in other headers, not here), and will get automatically imported to `em::Math` and `em`.
// NOTE: It's recommended to `using` stuff into `namespace Common` instead of declaring it here, to keep the full names sane. But this is optional.
//
// And then the user can do `using namespace em::Common;` or `using namespace em::Math::Common;` to import the popular parts
//   into their (global?) namespace.


// Prevent IWYU from warning that this header is unused:
// IWYU pragma: always_keep

namespace em
{
    namespace Math::inline Common {}

    inline namespace Common
    {
        using namespace Math::Common;
    }
}
