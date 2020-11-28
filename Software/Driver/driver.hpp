
#pragma once

#include <openvr_driver.h>


typedef struct{
	float xyzw[4];
	float pos_xyz[3];
} position3d;



extern "C" __declspec(dllexport) void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode);
 