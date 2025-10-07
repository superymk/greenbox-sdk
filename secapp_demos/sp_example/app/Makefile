# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

APP_LFLAGS = -pthread

ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	APP_LFLAGS += -lDbgHelp

	# Control OS sound mute/unmute
	APP_LFLAGS += -lole32 -loleaut32 -luuid
endif

current_dir = $(shell pwd)
APP_INCLUDE += -I$(current_dir)/../include -I$(LIBDRV_USB2_PUBLIC_HEADERS) -I$(SECBASE_PUBLIC_HEADERS)

.PHONY: all
all: $(OBJECTS) static
	
%.o: %.S Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<
	
.PHONY: static
static:
	$(CC) -o $(DIST_SAP_PATH)/app $(OBJECTS) $(BUILD_PATH)/app-libs.o $(APP_LFLAGS)
	$(STRIP) $(DIST_SAP_PATH)/app
	$(CC) -o $(DIST_PRIV_SAP_PATH)/app $(OBJECTS) $(BUILD_PATH)/app-libs.o $(APP_LFLAGS)

.PHONY: shared
shared:
	$(CC) -o $(DIST_SAP_PATH)/app $(OBJECTS) $(APP_LFLAGS) -L$(DIST_SAP_PATH) -l:app-libs.so
	$(STRIP) $(DIST_SAP_PATH)/app
	$(CC) -o $(DIST_PRIV_SAP_PATH)/app $(OBJECTS) $(APP_LFLAGS) -L$(DIST_PRIV_SAP_PATH) -l:app-libs.so
	
clean:
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: