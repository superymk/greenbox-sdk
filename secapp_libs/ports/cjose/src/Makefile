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

INCLUDE_PATH += -I./include -I../include -I$(SAP_PORT_LIBS_DIST_DIR)/include

SAP_PORT_LIBS_CFLAGS += -DHAVE_STDINT_H

.PHONY: all
all: $(OBJECTS)
	$(AR) -rcs libcjose.a *.o
	

%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(SAP_PORT_LIBS_CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(SAP_PORT_LIBS_CFLAGS) -o $@ $<

.PHONY: src
src:
	@$(MAKE) -C ./src
	
.PHONY: install
install:
	@$(CP) -f ./libcjose.a $(SAP_PORT_LIBS_DIST_DIR)/lib/
	
.PHONY: clean
clean:
	@$(RM) *.o libcjose.a
	
# Define a phony target that always runs
.PHONY: FORCE
FORCE: