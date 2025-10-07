# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

APP_INCLUDE += -I../include -I$(SECSRV_INIT_PUBLIC_HEADERS) -I$(SECOS_PUBLIC_HEADERS)

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o lib-hash.o $(OBJECTS)
	
%.o: %.S Makefile
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<
	
.PHONY: clean
clean:
	$(RM) *.o