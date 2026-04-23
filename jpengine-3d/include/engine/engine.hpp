#pragma once

// Umbrella header — pulls in the full public engine API.
// User code should prefer this include over the individual headers.
//
// The IWYU "export" pragmas tell clangd/include-what-you-use that these
// re-exports are intentional — they silence the "included header is not
// used directly" warning in this file.

#include "engine/src/application.hpp"             // IWYU pragma: export
#include "engine/src/engine.hpp"                  // IWYU pragma: export
#include "engine/src/graphics/shader-program.hpp" // IWYU pragma: export
#include "engine/src/input/input-manager.hpp"     // IWYU pragma: export
