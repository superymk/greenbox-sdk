# // SPDX-License-Identifier: GPL-2.0-only
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

VER=$(shell lsb_release -sr)

ifeq ($(VER), 12.04)
	CC := gcc-4.6
endif

obj-m := secagent.o

# Compile files
secagent-objs := \
../utils/utils.o \
../devs/pci.o \
../devs/ps2.o \
../devs/ehci.o \
../devs/xhci_shared.o \
../devs/gpu/intel/gen9.o \
../devs/gpu/intel/i965_ops.o \
../devs/gpu/intel/ilk.o \
../devs/gpu/gpu.o \
../devs/gpu/gpu_debug.o \
../common/apic.o \
../common/mm.o \
../common/intr.o \
../common/comm_sos.o \
../common/comm_sos_handler.o \
../common/ioctl_handler.o \
../common/perf.o \
../common/app_notif_mgr.o \
../common/sa_process.o \
../common/secagent.o \
../common/secbase.o \
../common/sos_mm_mgr.o \
../common/sos_sp_params.o \
../common/sos.o \
../common/stl_dlist.o \
../common/untrusted_loader_info.o \
../common/untrusted_loader_utils.o \
../common/debug.o \
../common/arch/x86/sa_arch.o \
x86/linux_cpu.o \
x86/linux_mm.o \
x86/linux_intr.o \
x86/linux_event_notify.o \
x86/linux_funcs.o \
driver_entry.o \

ifeq ($(TARGET_PLATFORM), i386)
	secagent-objs += ../common/arch/x86/i386/i386_apic.o
else ifeq ($(TARGET_PLATFORM), amd64)
	secagent-objs += ../common/arch/x86/amd64/amd64_apic.o
endif

# Compile flags
ifeq ($(TARGET_PLATFORM), i386)
	EXTRA_CFLAGS += -D__x86__ -D__i386__
else ifeq ($(TARGET_PLATFORM), amd64)
	EXTRA_CFLAGS += -D__x86__ -D__amd64__
endif

ifeq ($(DEBUG), y)
	EXTRA_CFLAGS += -g -DDEBUG
else ifeq ($(DEBUG), n)
	EXTRA_CFLAGS += -O2 -DNDEBUG
endif

ifdef XMHF_LOG_BUF_SIZE
	EXTRA_CFLAGS += -DXMHF_LOG_BUF_SIZE=$(XMHF_LOG_BUF_SIZE)
endif

SECAGENT_HEADERS := $(SECAGENT_PATH)/include

EXTRA_CFLAGS += -I$(APP_HEADERS)/secagent -I$(SECBASE_PUBLIC_HEADERS) -I$(SECOS_PUBLIC_HEADERS) -I$(SECAGENT_HEADERS) -I./include -I.

KERNEL_SRC   := /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

#~ .PHONY: all clean
all:
	make -C $(KERNEL_SRC) M=$(PWD) modules
	$(CP) *.ko $(DIST_SOS_PATH)
	$(CP) *.ko $(DIST_PRIV_SOS_PATH)

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean
	@$(RM) ../common/*.cmd
	@$(RM) $(secagent-objs)

