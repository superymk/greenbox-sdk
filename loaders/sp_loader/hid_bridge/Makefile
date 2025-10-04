# /*
#  * -------------------------------------------------------------------------------------
#  * Carnegie Mellon University
#  * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
#  * 
#  * This software is the proprietary information of Carnegie Mellon University.
#  * Use is subject to license terms.
#  * 
#  * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
#  * written consent of Carnegie Mellon University. is strictly prohibited.
#  * -------------------------------------------------------------------------------------
#  * 
#  * Authors: Miao Yu
#  *          Virgil Gligor
#  */

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