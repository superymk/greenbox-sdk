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

PAL_CFLAGS := $(filter-out -Werror,$(PAL_CFLAGS))
CFLAGS := $(filter-out -Werror,$(CFLAGS))

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o lib-ui-control.o $(OBJECTS)

%.o: %.S Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(CFLAGS) $(PAL_CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(PAL_INCLUDE) $(CFLAGS) $(PAL_CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: