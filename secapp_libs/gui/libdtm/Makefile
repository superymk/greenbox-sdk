# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

current_dir = $(shell pwd)
INCLUDE_PATH += -I$(current_dir)/include -I$(SECSRV_DTM_PUBLIC_HEADERS) -I$(LIBDRV_HID_PUBLIC_HEADERS)

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o sap-libdtm.o $(OBJECTS)

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