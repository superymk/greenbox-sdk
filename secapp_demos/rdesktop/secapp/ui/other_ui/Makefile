# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o other-ui.o $(OBJECTS)

%.o: %.S Makefile
	$(CC) -c $(PAL_INCLUDE) $(CFLAGS) $(PAL_CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile
	$(CC) -c $(PAL_INCLUDE) $(CFLAGS) $(PAL_CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@$(RM) *.o