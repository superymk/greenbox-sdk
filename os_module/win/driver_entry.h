// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#pragma once

#include <ntddk.h>
#include <wdf.h>

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <wdmsec.h> // for SDDLs
#include "wpp_trace.h" // contains macros for WPP tracing

#define SA_NTDEVICE_NAME_STRING      L"\\Device\\SecAgent"
#define SA_WDFDEVICE_NAME_STRING      L"\\Device\\SecAgentWDF"
#define SA_SYMBOLIC_NAME_STRING     L"\\DosDevices\\SecAgent"

extern void win_HALT(void);
#define assert(rsn, msg) do { if(!rsn) {	\
	DbgPrint("[%s:%d] Panic: ",__FILE__,__LINE__); \
	DbgPrint msg; \
	win_HALT(); }} while(0) 

typedef struct _CONTROL_DEVICE_EXTENSION {

    HANDLE   FileHandle; // Store your control data here

} CONTROL_DEVICE_EXTENSION, *PCONTROL_DEVICE_EXTENSION;

extern NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
);
	
extern NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject);