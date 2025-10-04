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

APP_LFLAGS = -pthread

ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	APP_LFLAGS += -lDbgHelp

	# Control OS sound mute/unmute
	APP_LFLAGS += -lole32 -loleaut32 -luuid
endif

APP_INCLUDE += -I$(LIBDRV_USB2_PUBLIC_HEADERS) -I$(SECBASE_PUBLIC_HEADERS)

# Debug Options
ifeq ($(DEBUG),y)
	# Debug - Link Address Sanitizer if <CONFIG_DEBUG_LOADERS_MEMLEAK> is set.
	ifeq ($(CONFIG_DEBUG_LOADERS_MEMLEAK), y)
		CFLAGS += -fsanitize=address
		APP_LFLAGS += -fsanitize=address
	endif
endif
 
.PHONY: all
all: $(OBJECTS) static copy_cfg
	
%.o: %.S Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<

.PHONY: copy_cfg
copy_cfg:
	@$(CP) -f ./*.json $(DIST_SAP_PATH)/
	@$(CP) -f ./*.json $(DIST_PRIV_SAP_PATH)/

.PHONY: static
static:
	$(CC) -o $(DIST_SAP_PATH)/sl_rfiles $(OBJECTS) $(BUILD_PATH)/app-libs.o $(APP_LFLAGS)
	$(STRIP) $(DIST_SAP_PATH)/sl_rfiles
	$(CC) -o $(DIST_PRIV_SAP_PATH)/sl_rfiles $(OBJECTS) $(BUILD_PATH)/app-libs.o $(APP_LFLAGS)

.PHONY: shared
shared:
	$(CC) -o $(DIST_SAP_PATH)/sl_rfiles $(OBJECTS) $(APP_LFLAGS) -L$(DIST_SAP_PATH) -l:app-libs.so
	$(STRIP) $(DIST_SAP_PATH)/sl_rfiles
	$(CC) -o $(DIST_PRIV_SAP_PATH)/sl_rfiles $(OBJECTS) $(APP_LFLAGS) -L$(DIST_PRIV_SAP_PATH) -l:app-libs.so
	
clean:
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: