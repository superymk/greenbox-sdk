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

#include "ntddk.h"
#include <wdf.h>
//#include "main.h"

/*NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    PAGED_CODE();

    DbgPrint("Hello World!");
    //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Hello World!"));

    pDriverObject->DriverUnload = DriverUnload;

    return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pDriverObject);
    PAGED_CODE();

    DbgPrint("Bye World!");
    //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bye World!"));

    return STATUS_SUCCESS;
}*/