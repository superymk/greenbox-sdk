# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

.PHONY: all
all: sl_rpc $(OBJECTS)
	$(LD) -r -o app-ipc.o $(OBJECTS) ./sl_rpc/app-sl-rpc.o

%.o: %.S Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<
	
.PHONY: sl_rpc
sl_rpc:
	@$(MAKE) -C sl_rpc all

clean:
	cd sl_rpc && $(MAKE) -w clean
	rm -f *.o