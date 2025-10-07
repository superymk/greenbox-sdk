# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

APP_INCLUDE += -I../include -I$(SECSRV_INIT_PUBLIC_HEADERS) -I$(SECOS_PUBLIC_HEADERS)

# For LibUSB
APP_INCLUDE += $(shell echo `pkg-config libusb-1.0 --cflags`)

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o sl-dev.o $(OBJECTS)

%.o: %.S Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: