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
INCLUDE_PATH += -I$(current_dir)/include 
INCLUDE_PATH += -I$(IOSM_USB3_PUBLIC_HEADERS)

.PHONY: all
all: utils mgr xhci $(OBJECTS)
	@$(ECHO) "Building the LibDrv-USB"  
	$(LD) -r -o sap-libdrv-usb2.o ./utils/libusb2-utils.o ./mgr/libusb2-mgrs.o ./xhci/lib-iosm-xhci.o $(OBJECTS)
	
%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

# .PHONY: ehci
# ehci:
# 	@$(MAKE) -C ./ehci

.PHONY: xhci
xhci:
	@$(MAKE) -C ./xhci

.PHONY: mgr
mgr:
	@$(MAKE) -C ./mgr

.PHONY: utils
utils:
	@$(MAKE) -C ./utils

.PHONY: clean
clean:
	# cd ehci && $(MAKE) -w clean
	cd mgr && $(MAKE) -w clean
	cd xhci && $(MAKE) -w clean
	cd utils && $(MAKE) -w clean
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: