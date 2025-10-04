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

INCLUDE_PATH += -I../hw_ctrls -I../hw_ctrls/usb -I../hw_ctrls/pci

dist_dir := $(abspath $(CURDIR)/..)/dist
pc       := $(dist_dir)/lib/pkgconfig/jansson.pc

.PHONY: all
all: src

%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: src
src:
	@$(MAKE) -C ./src all
	
.PHONY: install-lib
install-lib:
	@$(MAKE) -C ./src install

.PHONY: install-pkgconfig
install-pkgconfig:
	$(MKDIR) $(SAP_PORT_LIBS_DIST_DIR)/lib/pkgconfig
	@$(CP) -f ./jansson.pc $(SAP_PORT_LIBS_DIST_DIR)/lib/pkgconfig/
	sed -i 's|^prefix=.*|prefix=$(dist_dir)|' '$(pc)'

.PHONY: install
install: install-lib install-pkgconfig
	
.PHONY: clean
clean:
	cd src && $(MAKE) -w clean
	
# Define a phony target that always runs
.PHONY: FORCE
FORCE: