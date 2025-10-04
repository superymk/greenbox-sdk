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
INCLUDE_PATH += -I$(current_dir)/include -I$(LIBDRV_USB2_PUBLIC_HEADERS) -I$(IOSM_USB3_PUBLIC_HEADERS) -I$(IOSM_PS2_PUBLIC_HEADERS)

CFLAGS += -Wno-sign-compare

.PHONY: all
all: $(OBJECTS)
	@$(ECHO) "Building LibDrv-HID"  
	$(LD) -r -o sap-libdrv-hid.o $(OBJECTS)
	
%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: