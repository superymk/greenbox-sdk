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