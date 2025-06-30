/*
Copyright (c) 2025 TrueSmartTech OÜ  
All rights reserved.  

This code is proprietary and confidential. Redistribution, modification, or commercial use without explicit permission is strictly prohibited.  

Version 5.0.0
Author: TrueSmartTech OÜ
www.truesmarttech.com
*/

#ifndef tstVariables_h
#define tstVariables_h

#include "tst_library.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*TSTVARIABLESSTART*/

#define TSTNAME "MyDevice"
#define TSTMAXSIZE 100

typedef struct __attribute__((packed)) {
	uint32_t      core0Count;
	uint32_t      core1Count;
} TST_Variables;

/*TSTVARIABLESEND*/

extern TST_Variables TST_V;

// Protocol Configuration
static const TST_StructConfig TST_Struct = {
    .name = "TST_Variables",
    .structName = "TST_Variables",
    .pStruct = &TST_V,
    .sStruct = sizeof(TST_V)
};

static const TST_InterfaceConfig TST_Interface = {
    .interface = "SERIAL", 
    .maxSize = TSTMAXSIZE
};

static const TST_DeviceConfig TST_Device = {
    .name = "MyDevice",
    .pInterfaces = &TST_Interface,
    .nInterfaces = sizeof(TST_Interface),
    .pStructs = &TST_Struct,
    .nStructs = sizeof(TST_Struct)
};

#ifdef __cplusplus
}
#endif

#endif
