// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* SPDX-License-Identifier: BSD-2 */

#ifndef _TCG2_PROTOCOL_H
#define _TCG2_PROTOCOL_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <os/os_headers.h>
#include <app_types.h>

#ifndef __ASSEMBLY__

#define TPM_MAX_PCR     (PLATFORM_PCR)

typedef uint32_t TCG_PCRINDEX;
typedef uint32_t TCG_EVENTTYPE;

#define PACKED __attribute__((packed))

typedef struct {
  uint16_t AlgorithmId;
  uint8_t Digest[];
} PACKED TCG_DIGEST2;

typedef struct {
  uint32_t EventSize;
  uint8_t Event [];
} PACKED TCG_EVENT2;

typedef struct {
  TCG_PCRINDEX PCRIndex;
  TCG_EVENTTYPE EventType;
  uint32_t DigestCount;
  TCG_DIGEST2 Digests[];
 /* TCG_EVENT2 comes next */
} PACKED TCG_EVENT_HEADER2;

//
// Standard event types
//
#define EV_PREBOOT_CERT             ((TCG_EVENTTYPE) 0x00000000)
#define EV_POST_CODE                ((TCG_EVENTTYPE) 0x00000001)
#define EV_NO_ACTION                ((TCG_EVENTTYPE) 0x00000003)
#define EV_SEPARATOR                ((TCG_EVENTTYPE) 0x00000004)
#define EV_ACTION                   ((TCG_EVENTTYPE) 0x00000005)
#define EV_EVENT_TAG                ((TCG_EVENTTYPE) 0x00000006)
#define EV_S_CRTM_CONTENTS          ((TCG_EVENTTYPE) 0x00000007)
#define EV_S_CRTM_VERSION           ((TCG_EVENTTYPE) 0x00000008)
#define EV_CPU_MICROCODE            ((TCG_EVENTTYPE) 0x00000009)
#define EV_PLATFORM_CONFIG_FLAGS    ((TCG_EVENTTYPE) 0x0000000A)
#define EV_TABLE_OF_DEVICES         ((TCG_EVENTTYPE) 0x0000000B)
#define EV_COMPACT_HASH             ((TCG_EVENTTYPE) 0x0000000C)
#define EV_NONHOST_CODE             ((TCG_EVENTTYPE) 0x0000000F)
#define EV_NONHOST_CONFIG           ((TCG_EVENTTYPE) 0x00000010)
#define EV_NONHOST_INFO             ((TCG_EVENTTYPE) 0x00000011)
#define EV_OMIT_BOOT_DEVICE_EVENTS  ((TCG_EVENTTYPE) 0x00000012)

//
// EFI specific event types
//
#define EV_EFI_EVENT_BASE                 ((TCG_EVENTTYPE) 0x80000000)
#define EV_EFI_VARIABLE_DRIVER_CONFIG     (EV_EFI_EVENT_BASE + 1)
#define EV_EFI_VARIABLE_BOOT              (EV_EFI_EVENT_BASE + 2)
#define EV_EFI_BOOT_SERVICES_APPLICATION  (EV_EFI_EVENT_BASE + 3)
#define EV_EFI_BOOT_SERVICES_DRIVER       (EV_EFI_EVENT_BASE + 4)
#define EV_EFI_RUNTIME_SERVICES_DRIVER    (EV_EFI_EVENT_BASE + 5)
#define EV_EFI_GPT_EVENT                  (EV_EFI_EVENT_BASE + 6)
#define EV_EFI_ACTION                     (EV_EFI_EVENT_BASE + 7)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB     (EV_EFI_EVENT_BASE + 8)
#define EV_EFI_HANDOFF_TABLES             (EV_EFI_EVENT_BASE + 9)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB2    (EV_EFI_EVENT_BASE + 0xA)
#define EV_EFI_HANDOFF_TABLES2            (EV_EFI_EVENT_BASE + 0xB)
#define EV_EFI_HCRTM_EVENT                (EV_EFI_EVENT_BASE + 0x10)
#define EV_EFI_VARIABLE_AUTHORITY         (EV_EFI_EVENT_BASE + 0xE0)
#define EV_EFI_SPDM_FIRMWARE_BLOB         (EV_EFI_EVENT_BASE + 0xE1)
#define EV_EFI_SPDM_FIRMWARE_CONFIG       (EV_EFI_EVENT_BASE + 0xE2)
#define EV_EFI_SPDM_DEVICE_BLOB           EV_EFI_SPDM_FIRMWARE_BLOB
#define EV_EFI_SPDM_DEVICE_CONFIG         EV_EFI_SPDM_FIRMWARE_CONFIG


#endif // __ASSEMBLY__
#endif // _TCG2_PROTOCOL_H