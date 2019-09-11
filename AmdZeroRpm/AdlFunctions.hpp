#pragma once

#include "adl_defines.h"
#include "adl_sdk.h"

#define ADL_IMPORT __declspec(dllimport)

extern "C" {
ADL_IMPORT int ADL2_Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback,
                                        int iEnumConnectedAdapters,
                                        ADL_CONTEXT_HANDLE *context);

ADL_IMPORT int ADL2_Main_Control_Destroy(ADL_CONTEXT_HANDLE context);

ADL_IMPORT int ADL2_Adapter_Primary_Get(ADL_CONTEXT_HANDLE context,
                                        int *lpPrimaryAdapterIndex);

ADL_IMPORT int ADL2_OverdriveN_ZeroRPMFan_Get(ADL_CONTEXT_HANDLE context,
                                              int iAdapterIndex, int *lpSupport,
                                              int *lpCurrentValue,
                                              int *lpDefaultValue);

ADL_IMPORT int ADL2_OverdriveN_ZeroRPMFan_Set(ADL_CONTEXT_HANDLE context,
                                              int iAdapterIndex,
                                              int currentValue);
}

void *__stdcall AdlMemoryAllocator(int bytes);

void __stdcall AdlMemoryDeallocator(void **buffer);
