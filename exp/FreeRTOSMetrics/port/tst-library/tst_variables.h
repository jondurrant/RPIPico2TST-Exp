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
#define TSTMAXSIZE 256

typedef struct __attribute__((packed)) {
    // Application variables
    int32_t variable1;  // Example variable

    // System/monitoring variables
    uint32_t heap_free;
    uint32_t heap_min_ever;
    uint32_t task_count;
    uint32_t led_status; // 0=off, 1=on
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