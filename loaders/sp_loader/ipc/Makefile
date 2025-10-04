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