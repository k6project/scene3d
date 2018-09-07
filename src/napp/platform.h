#pragma once

#if defined(_NAPP_WINAPI_)
#define NAPP_PLATFORM_W32
#elif defined(_NAPP_WRL_)
#define NAPP_PLATFORM_WRL
#elif defined(_NAPP_MAC_)
#else
#error Undefined build platform
#endif
