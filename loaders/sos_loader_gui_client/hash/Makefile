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