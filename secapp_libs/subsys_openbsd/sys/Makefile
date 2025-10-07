# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

ifeq ($(TARGET_PLATFORM), i386)
	C_SOURCES = $(wildcard ./arch/i386/i386/*.c)
	AS_SOURCES = $(wildcard ./arch/i386/i386/*.S)
else ifeq ($(TARGET_PLATFORM), amd64)
	C_SOURCES = $(wildcard ./arch/amd64/amd64/*.c)
	AS_SOURCES = $(wildcard ./arch/amd64/amd64/*.S)
endif

C_SOURCES += $(wildcard ./kern/*.c)
AS_SOURCES += $(wildcard ./kern/*.S)

OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o arch.o $(OBJECTS)

%.o: %.S Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<


.PHONY: clean
clean:
	@$(RM) *.o