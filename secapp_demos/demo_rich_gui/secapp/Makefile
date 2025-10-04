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
PAL_INCLUDE += -I$(current_dir)/include -I$(current_dir)/../include \
				-I$(SECSRV_DTM_PUBLIC_HEADERS) -I$(LIBDRV_HID_PUBLIC_HEADERS) -I$(LIBGUI_BASEDIR)

PAL_CFLAGS += -Wno-sign-compare

# [NOTE] We must link <$(BUILD_PATH)/pal-libs.o>, because SecApps do not have GREENBOX_VERSION_NUMBER without linking 
# <pal-libs.o>. SecOS always checks <GREENBOX_VERSION_NUMBER> of trusted SecProcesses to prevent their rollback attacks. 
# [TODO][Issue 134] Move <GREENBOX_VERSION_NUMBER> in newlib.
.PHONY: all
all: $(OBJECTS)
	$(LD) -T $(SAP_LIBS_BASEDIR)/secapp.lds -e sp_entry -o $(DIST_SAP_PATH)/demo_sp.sap $(PAL_LFLAGS) $(OBJECTS) $(BUILD_PATH)/pal-libs.o $(PAL_LIBC_LIB)
	$(STRIP) $(DIST_SAP_PATH)/demo_sp.sap
	$(LD) -T $(SAP_LIBS_BASEDIR)/secapp.lds -e sp_entry -o $(DIST_PRIV_SAP_PATH)/demo_sp.sap $(PAL_LFLAGS) $(OBJECTS) $(BUILD_PATH)/pal-libs.o $(PAL_LIBC_LIB)
	
%.o: %.S Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(PAL_CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(PAL_CFLAGS) -o $@ $<
	
clean:
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: