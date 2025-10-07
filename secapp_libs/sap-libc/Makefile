# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

current_dir = $(shell pwd)
INCLUDE_PATH += -I$(current_dir)/include

.PHONY: all
all: stl $(OBJECTS)
	@$(ECHO) "Building the PAL-LibC"  
	$(LD) -r -o sap-libc.o ./stl/sap-libc-stl.o $(OBJECTS)

.PHONY: stl
stl:
	@$(MAKE) -C ./stl
	
%.o: %.S Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	cd stl && $(MAKE) -w clean
	@$(RM) *.o