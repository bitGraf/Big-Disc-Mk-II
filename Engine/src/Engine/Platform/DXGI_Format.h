#include "Engine/Defines.h"

#if RH_PLATFORM_WINDOWS
#include <dxgiformat.h>
#else
// if we are not on windows, we recreate the DXGI_FORMAT enum for ease of use.
#error Implement DXGI_FORMAT!!!
#endif