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

include $(SAP_LIBS_BASEDIR)/sap_compile.mk

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

current_dir = $(shell pwd)

PAL_INCLUDE += -I$(current_dir)/../include -I$(current_dir)/include -I$(current_dir) \
				-I$(current_dir)/libvncserver/include \
				-I$(SECSRV_DTM_PUBLIC_HEADERS) -I$(LIBDRV_HID_PUBLIC_HEADERS) -I$(LIBGUI_BASEDIR)

PAL_INCLUDE += -I$(SAP_PORT_LIBS_DIST_DIR)/include


# Libjpeg-turbo CFLAGS
LIB_CFLAGS = $(PAL_INCLUDE) $(PAL_CFLAGS)
LIB_CFLAGS := $(filter-out -Werror,$(LIB_CFLAGS))

SAP_LIBS = -L$(SAP_PORT_LIBS_DIST_DIR)/lib -L$(SAP_PORT_LIBS_DIST_DIR)/lib64 -loauth2 -lcjose -ljansson -lcurl -lssl -lcrypto -lz

# [NOTE] We must link <$(BUILD_PATH)/pal-libs.o>, because SecApps do not have GREENBOX_VERSION_NUMBER without linking 
# <pal-libs.o>. SecOS always checks <GREENBOX_VERSION_NUMBER> of trusted SecProcesses to prevent their rollback attacks. 
# [TODO][Issue 134] Move <GREENBOX_VERSION_NUMBER> in newlib.
.PHONY: all
all: protocols ui $(OBJECTS)
	$(LD) -T $(SAP_LIBS_BASEDIR)/secapp.lds -e sp_entry -o $(DIST_SAP_PATH)/rfiles.sap $(PAL_LFLAGS) $(OBJECTS) ./ui/lib-ui.o ./protocols/lib-rfiles-protocols.o $(SAP_LIBS) $(BUILD_PATH)/pal-libs.o $(PAL_LIBC_LIB)
	$(STRIP) $(DIST_SAP_PATH)/rfiles.sap
	$(LD) -T $(SAP_LIBS_BASEDIR)/secapp.lds -e sp_entry -o $(DIST_PRIV_SAP_PATH)/rfiles.sap $(PAL_LFLAGS) $(OBJECTS) ./ui/lib-ui.o ./protocols/lib-rfiles-protocols.o $(SAP_LIBS) $(BUILD_PATH)/pal-libs.o $(PAL_LIBC_LIB)


%.o: %.S Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(PAL_CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(PAL_CFLAGS) -o $@ $<

.PHONY: ui
ui:
	@$(MAKE) -C ./ui

.PHONY: protocols
protocols:
	@$(MAKE) -C ./protocols

.PHONY: clean
clean:
	cd ui && $(MAKE) -w clean
	cd protocols && $(MAKE) -w clean
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: