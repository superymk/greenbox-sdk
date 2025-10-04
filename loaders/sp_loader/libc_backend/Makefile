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