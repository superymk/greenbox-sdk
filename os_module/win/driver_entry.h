/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

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