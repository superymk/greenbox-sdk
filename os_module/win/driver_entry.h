// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#pragma once

extern void win_HALT(void);

#define assert(rsn, msg) do { if(!rsn) {	\
	DbgPrint("[%s:%d] Panic: ",__FILE__,__LINE__); \
	DbgPrint msg; \
	win_HALT(); }} while(0) 
	
NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
);