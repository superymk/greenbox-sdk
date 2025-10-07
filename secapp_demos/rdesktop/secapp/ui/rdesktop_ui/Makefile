# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard ./export/*.c)
C_SOURCES += $(wildcard ./export/components/*.c)
C_SOURCES += $(wildcard ./export/fonts/*.c)
C_SOURCES += $(wildcard ./export/images/*.c)
C_SOURCES += $(wildcard ./export/screens/*.c)
AS_SOURCES = $(wildcard ./export/*.S)

OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

PAL_CFLAGS := $(filter-out -Werror,$(PAL_CFLAGS))
CFLAGS := $(filter-out -Werror,$(CFLAGS))

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o lib-rdesktop-ui.o $(OBJECTS)

%.o: %.S Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(CFLAGS) $(PAL_CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(CFLAGS) $(PAL_CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@$(RM) ./export/components/*.o
	@$(RM) ./export/fonts/*.o
	@$(RM) ./export/images/*.o
	@$(RM) ./export/screens/*.o
	@$(RM) ./export/*.o
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: