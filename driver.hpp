
#pragma once

#include <openvr_driver.h>

extern "C" __declspec(dllexport) void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode);
 