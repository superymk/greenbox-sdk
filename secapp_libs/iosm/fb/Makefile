# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

current_dir = $(shell pwd)
INCLUDE_PATH += -I$(current_dir)/include -I$(IOSM_FB_PUBLIC_HEADERS)

.PHONY: all
all: mgr $(OBJECTS)
	@$(ECHO) "Building LibDrv-FB"  
	$(LD) -r -o sap-libdrv-fb.o $(OBJECTS) ./mgr/libfb-mgrs.o
	
%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: mgr
mgr:
	@$(MAKE) -C ./mgr

.PHONY: clean
clean:
	cd mgr && $(MAKE) -w clean
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: