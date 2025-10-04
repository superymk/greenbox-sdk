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

# Windows service cannot start SOSLoader directly because it waits for socket connections and does not exit. Thus, 
# Windows service needs to run an SOSLoader wrapper instead.

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

current_dir = $(shell pwd)
APP_INCLUDE += -I$(current_dir)/include

# Embed SHA256(sos_loader) into the wrapper.
## Path to the sos_loader executable
SOS_LOADER_EXE = $(DIST_SOS_PATH)/sos_loader.exe

## Function to compute and format SHA256 checksum
define SOS_LOADER_COMPUTE_SHA256
	sha256sum $(SOS_LOADER_EXE) | \
	sed -e 's/ .*//; s/\([0-9a-f][0-9a-f]\)/0x\1, /g; s/, $$//'
endef

## Compute the SHA256 checksum and format it
SOS_LOADER_SHA256 = $(shell $(SOS_LOADER_COMPUTE_SHA256))

## Add the formatted SHA256 checksum as a -D option
CFLAGS += -DSOS_LOADER_SHA256="$(SOS_LOADER_SHA256)"


.PHONY: all
all: hash $(OBJECTS)
ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	$(CC) -o $(DIST_SOS_PATH)/sos_loader_server_win_wrapper $(OBJECTS) ./hash/sl-hash.o
	$(STRIP) $(DIST_SOS_PATH)/sos_loader_server_win_wrapper
	$(CC) -o $(DIST_PRIV_SOS_PATH)/sos_loader_server_win_wrapper $(OBJECTS) ./hash/sl-hash.o
endif

%.o: %.S Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<

.PHONY: hash
hash:
	@$(MAKE) -C ./hash

.PHONY: clean
clean:
	cd hash && $(MAKE) -w clean
	$(RM) *.o sos_loader_server_win_wrapper

# Define a phony target that always runs
.PHONY: FORCE
FORCE: