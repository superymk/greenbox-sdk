# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

INCLUDE_PATH += -I./

SAP_PORT_LIBS_CFLAGS += -DHAVE_STDINT_H

.PHONY: all
all: $(OBJECTS)
	$(AR) -rcs libjansson.a *.o
	

%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(SAP_PORT_LIBS_CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(SAP_PORT_LIBS_CFLAGS) -o $@ $<

.PHONY: src
src:
	@$(MAKE) -C ./src
	
.PHONY: install
install:
	@$(CP) -f ./jansson.h $(SAP_PORT_LIBS_DIST_DIR)/include/
	@$(CP) -f ./jansson_config.h $(SAP_PORT_LIBS_DIST_DIR)/include/
	@$(CP) -f ./libjansson.a $(SAP_PORT_LIBS_DIST_DIR)/lib/
	
.PHONY: clean
clean:
	@$(RM) *.o libjansson.a
	
# Define a phony target that always runs
.PHONY: FORCE
FORCE: