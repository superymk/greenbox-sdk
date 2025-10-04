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

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

current_dir = $(shell pwd)

# LD flags for RPC to SOSLoader Server
# MSYS2_LDFLAGS_RPC = -lWs2_32 -lMswsock -lAdvApi32

# Debug Options
ifeq ($(DEBUG),y)
	# Debug - Link Address Sanitizer if <CONFIG_DEBUG_LOADERS_MEMLEAK> is set.
	ifeq ($(CONFIG_DEBUG_LOADERS_MEMLEAK), y)
		CFLAGS += -fsanitize=address
	endif
endif

INCLUDE_PATH += -I$(LOADERS_EXTERNAL_BASEDIR) -I$(APP_HEADERS)
INCLUDE_PATH += -I$(SECOS_PUBLIC_HEADERS) -I$(SECAGENT_PUBLIC_HEADERS) -I$(SECOS_LOADER_PUBLIC_HEADERS) -I$(SECBASE_PUBLIC_HEADERS) 
INCLUDE_PATH += -I$(current_dir)/libc_backend/include
export INCLUDE_PATH

.PHONY: all
all: c-json os_specific ipc hash gui-bridge hid-bridge tpm-bridge libc_backend $(OBJECTS) static
	

%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: static
static:
ifeq ($(OS), windows_msys2)
	$(LD) -r -o $(BUILD_PATH)/app-libs.o ./cJSON/*.o ./win/*.o $(OBJECTS) \
	./libc_backend/libc-backend.o ./ipc/app-ipc.o ./hash/sl-hash.o ./gui_bridge/app-gui-bridge.o ./hid_bridge/app-hid-bridge.o ./tpm_bridge/app-tpm-bridge.o \
	$(MSYS2_LDFLAGS_RPC)
else ifeq ($(OS), linux)
	$(LD) -r -o $(BUILD_PATH)/app-libs.o ./cJSON/*.o ./linux/*.o $(OBJECTS) \
	./libc_backend/libc-backend.o ./ipc/app-ipc.o ./hash/sl-hash.o ./gui_bridge/app-gui-bridge.o ./hid_bridge/app-hid-bridge.o ./tpm_bridge/app-tpm-bridge.o 
endif

.PHONY: shared
shared:
ifeq ($(OS), windows_msys2)
	$(CC) $(CFLAGS) ./cJSON/*.o ./win/*.o $(OBJECTS) ./libc_backend/libc-backend.o ./ipc/app-ipc.o ./hash/sl-hash.o ./gui_bridge/app-gui-bridge.o ./hid_bridge/app-hid-bridge.o ./tpm_bridge/app-tpm-bridge.o  -shared -o $(DIST_SAP_PATH)/app-libs.so
	$(STRIP) $(DIST_SAP_PATH)/app-libs.so
	$(CC) $(CFLAGS) ./cJSON/*.o ./win/*.o $(OBJECTS) ./libc_backend/libc-backend.o ./ipc/app-ipc.o ./hash/sl-hash.o ./gui_bridge/app-gui-bridge.o ./hid_bridge/app-hid-bridge.o ./tpm_bridge/app-tpm-bridge.o -shared -o $(DIST_PRIV_SAP_PATH)/app-libs.so
else ifeq ($(OS), linux)
	$(CC) $(CFLAGS) ./cJSON/*.o ./linux/*.o $(OBJECTS) ./libc_backend/libc-backend.o ./ipc/app-ipc.o ./hash/sl-hash.o ./gui_bridge/app-gui-bridge.o ./hid_bridge/app-hid-bridge.o ./tpm_bridge/app-tpm-bridge.o -shared -o $(DIST_SAP_PATH)/app-libs.so
	$(STRIP) $(DIST_SAP_PATH)/app-libs.so
	$(CC) $(CFLAGS) ./cJSON/*.o ./linux/*.o $(OBJECTS) ./libc_backend/libc-backend.o ./ipc/app-ipc.o ./hash/sl-hash.o ./gui_bridge/app-gui-bridge.o ./hid_bridge/app-hid-bridge.o ./tpm_bridge/app-tpm-bridge.o -shared -o $(DIST_PRIV_SAP_PATH)/app-libs.so
endif
	
.PHONY: c-json
c-json:
	@$(MAKE) -C ./cJSON static

.PHONY: ipc
ipc:
	@$(MAKE) -C ipc all

.PHONY: hash
hash:
	@$(MAKE) -C ./hash

.PHONY: gui-bridge
gui-bridge:
	@$(MAKE) -C gui_bridge all

.PHONY: hid-bridge
hid-bridge:
	@$(MAKE) -C hid_bridge all

.PHONY: tpm-bridge
tpm-bridge:
	@$(MAKE) -C tpm_bridge all
	
.PHONY: libc_backend
libc_backend:
	@$(MAKE) -C libc_backend all

.PHONY: os_specific
os_specific:
ifeq ($(OS), windows_msys2)
	@$(MAKE) -C win all
else ifeq ($(OS), linux)
	@$(MAKE) -C linux all
endif

.PHONY: clean
clean:
	cd cJSON && $(MAKE) -w clean
	cd linux && $(MAKE) -w clean
	cd ipc && $(MAKE) -w clean
	cd hash && $(MAKE) -w clean
	cd gui_bridge && $(MAKE) -w clean
	cd hid_bridge && $(MAKE) -w clean
	cd tpm_bridge && $(MAKE) -w clean
	cd libc_backend && $(MAKE) -w clean
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: