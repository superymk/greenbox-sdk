# /*
#  * -------------------------------------------------------------------------------------
#  * Carnegie Mellon University
#  * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
#  * 
#  * This software is the proprietary information of Carnegie Mellon University.
#  * Use is subject to license terms.
#  * 
#  * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
#  * written consent of Carnegie Mellon University. is strictly prohibited.
#  * -------------------------------------------------------------------------------------
#  * 
#  * Authors: Miao Yu
#  *          Virgil Gligor
#  */

SL_LFLAGS =

# Embed SHA256 of the gb_tpm tool <tools/tpmtools/gb_tpm> into the caller of the executable.
## Path to the gb_tpm executable
ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	GB_TPM_EXECUTABLE = $(DIST_TOOLS_PATH)/tpmtools/gb_tpm.exe
else ifeq ($(CONFIG_OS_LINUX), y)
	GB_TPM_EXECUTABLE = $(DIST_TOOLS_PATH)/tpmtools/gb_tpm
endif

## Function to compute and format SHA256 checksum
define GB_TPM_EXECUTABLE_COMPUTE_SHA256
	sha256sum $(GB_TPM_EXECUTABLE) | \
	sed -e 's/ .*//; s/\([0-9a-f][0-9a-f]\)/0x\1, /g; s/, $$//'
endef

## Compute the SHA256 checksum and format it
GB_TPM_EXECUTABLE_SHA256 = $(shell $(GB_TPM_EXECUTABLE_COMPUTE_SHA256))

## Add the formatted SHA256 checksum as a -D option
CFLAGS += -DGB_TPM_EXECUTABLE_SHA256="$(GB_TPM_EXECUTABLE_SHA256)"

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

APP_INCLUDE += -I./include -I$(LOADERS_EXTERNAL_BASEDIR) -I$(APP_HEADERS) -I$(SECAGENT_PUBLIC_HEADERS) -I$(SECOS_PUBLIC_HEADERS) -I$(SECBASE_PUBLIC_HEADERS)
APP_INCLUDE += -I$(IOSM_PCIE_PUBLIC_HEADERS) -I$(IOSM_USB3_PUBLIC_HEADERS) -I$(IOSM_FB_PUBLIC_HEADERS) 

SL_LFLAGS += -pthread

# # Additional SL_LFLAGS for wxWidgets
# ifeq ($(CONFIG_SOSLOADER_GUI), y)
# 	ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
# 		WX_FLAGS := $(shell echo `/mingw64/bin/wx-config --cxxflags --libs`)
# 	else ifeq ($(CONFIG_OS_LINUX), y)
# 		WX_FLAGS := $(shell echo `wx-config --cxxflags --libs`)
# 	endif
# 	SL_LFLAGS += $(WX_FLAGS)
# endif

# Additional SL_LFLAGS for LibUSB
SL_LFLAGS += $(shell echo `pkg-config libusb-1.0 --libs`)

ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	SL_LFLAGS += -L/ucrt64/lib -lcfgmgr32 -lsetupapi -lDbgHelp

	# Control OS sound mute/unmute
	SL_LFLAGS += -lole32 -loleaut32 -luuid
endif

# Debug Options
ifeq ($(DEBUG),y)
	# Debug - Link Address Sanitizer if <CONFIG_DEBUG_LOADERS_MEMLEAK> is set.
	ifeq ($(CONFIG_DEBUG_LOADERS_MEMLEAK), y)
		CFLAGS += -fsanitize=address
		SL_LFLAGS += -fsanitize=address
	endif
endif

.PHONY: all
ifeq ($(CONFIG_SOSLOADER_GUI), y)
all: DEBUG common dev secsrvs os UI cpp_link
else
all: DEBUG common dev secsrvs os UI gcc_link
endif
	
.PHONY: gcc_link
gcc_link:
	$(CC) -o $(DIST_SOS_PATH)/sos_loader $(OBJECTS) $(BUILD_PATH)/app-libs.o ./debug/debug.o ./dev/sl-dev.o ./secsrvs/sl-secsrvs.o ./common/sl-common.o  ./os-funcs.o ./ui/ui.o $(SL_LFLAGS)
	$(STRIP) $(DIST_SOS_PATH)/sos_loader
	$(CC) -o $(DIST_PRIV_SOS_PATH)/sos_loader $(OBJECTS) $(BUILD_PATH)/app-libs.o ./debug/debug.o ./dev/sl-dev.o ./secsrvs/sl-secsrvs.o ./common/sl-common.o ./os-funcs.o ./ui/ui.o $(SL_LFLAGS)
	
.PHONY: cpp_link
cpp_link:
ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	$(CXX) -o $(DIST_SOS_PATH)/sos_loader $(OBJECTS) $(BUILD_PATH)/app-libs.o ./debug/debug.o ./dev/sl-dev.o ./secsrvs/sl-secsrvs.o ./common/sl-common.o ./os-funcs.o ./ui/ui.o $(SL_LFLAGS)
	$(STRIP) $(DIST_SOS_PATH)/sos_loader
	$(CXX) -o $(DIST_PRIV_SOS_PATH)/sos_loader $(OBJECTS) $(BUILD_PATH)/app-libs.o ./debug/debug.o ./dev/sl-dev.o ./secsrvs/sl-secsrvs.o ./common/sl-common.o ./os-funcs.o ./ui/ui.o $(SL_LFLAGS)
else
	$(CXX) -o $(DIST_SOS_PATH)/sos_loader $(OBJECTS) $(BUILD_PATH)/app-libs.o ./debug/debug.o ./dev/sl-dev.o ./secsrvs/sl-secsrvs.o ./common/sl-common.o ./os-funcs.o ./ui/ui.o $(SL_LFLAGS)
	$(STRIP) $(DIST_SOS_PATH)/sos_loader
	$(CXX) -o $(DIST_PRIV_SOS_PATH)/sos_loader $(OBJECTS) $(BUILD_PATH)/app-libs.o ./debug/debug.o ./dev/sl-dev.o ./secsrvs/sl-secsrvs.o ./common/sl-common.o ./os-funcs.o ./ui/ui.o $(SL_LFLAGS)
endif
	
	
.PHONY: common
common:
	@$(MAKE) -C ./common

.PHONY: dev
dev:
	@$(MAKE) -C ./dev

.PHONY: secsrvs
secsrvs:
	@$(MAKE) -C ./secsrvs

.PHONY: os
os:
ifeq ($(OS), windows_msys2)
	@$(MAKE) -C ./win
	@$(LD) -r -o ./os-funcs.o ./win/*.o
else ifeq ($(OS), linux)
	@$(MAKE) -C ./linux
	@$(LD) -r -o ./os-funcs.o ./linux/*.o
endif
	
.PHONY: DEBUG
DEBUG:
	@$(MAKE) -C ./debug
	
.PHONY: UI
UI:
	@$(MAKE) -C ./ui

%.o: %.S Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	cd common && $(MAKE) -w clean
	cd dev && $(MAKE) -w clean
	cd secsrvs && $(MAKE) -w clean
	cd linux && $(MAKE) -w clean
	cd debug && $(MAKE) -w clean
	cd ui && $(MAKE) -w clean
	$(RM) *.o sos_loader

# Define a phony target that always runs
.PHONY: FORCE
FORCE: