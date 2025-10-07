# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

INCLUDE_PATH = -I. -I./include -I../include -I$(SECOS_PUBLIC_HEADERS) -I$(SECAGENT_PUBLIC_HEADERS) -I$(SECBASE_PUBLIC_HEADERS)

ifeq ($(CONFIG_OS_LINUX), y)
    CFLAGS += -Wall -Werror -fPIC
else ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
    CFLAGS += -Wall -Werror
endif

.PHONY: all
all: $(OBJECTS)
	@$(LD) -r -o libc-backend.o $(OBJECTS)
	
%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

clean:
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: