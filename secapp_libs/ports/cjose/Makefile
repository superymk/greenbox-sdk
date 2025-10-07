# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

INCLUDE_PATH += -I../hw_ctrls -I../hw_ctrls/usb -I../hw_ctrls/pci

dist_dir := $(abspath $(CURDIR)/..)/dist
pc       := $(dist_dir)/lib/pkgconfig/cjose.pc

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
	@$(CP) -rf ./include/cjose $(SAP_PORT_LIBS_DIST_DIR)/include/cjose

.PHONY: install-pkgconfig
install-pkgconfig:
	$(MKDIR) $(SAP_PORT_LIBS_DIST_DIR)/lib/pkgconfig
	@$(CP) -f ./cjose.pc $(SAP_PORT_LIBS_DIST_DIR)/lib/pkgconfig/
	sed -i 's|^prefix=.*|prefix=$(dist_dir)|' '$(pc)'

.PHONY: install
install: install-lib install-pkgconfig
	
.PHONY: clean
clean:
	cd src && $(MAKE) -w clean
	
# Define a phony target that always runs
.PHONY: FORCE
FORCE: