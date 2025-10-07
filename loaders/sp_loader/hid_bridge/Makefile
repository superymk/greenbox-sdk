# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

.PHONY: all
all: os_specific $(OBJECTS) static
	

%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: static
static:
ifeq ($(OS), windows_cygwin)
	$(LD) -r -o app-hid-bridge.o $(OBJECTS) ./win/*.o
else ifeq ($(OS), windows_msys2)
	$(LD) -r -o app-hid-bridge.o $(OBJECTS) ./win/*.o
else ifeq ($(OS), linux)
	$(LD) -r -o app-hid-bridge.o $(OBJECTS) ./linux/*.o
endif

.PHONY: os_specific
os_specific:
ifeq ($(OS), windows_cygwin)
	@$(MAKE) -C win all
else ifeq ($(OS), windows_msys2)
	@$(MAKE) -C win all
else ifeq ($(OS), linux)
	@$(MAKE) -C linux all
endif
	
clean:
	cd linux && $(MAKE) -w clean
	rm -f *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: