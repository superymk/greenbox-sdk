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

APP_INCLUDE += -I./include

.PHONY: all
all: linux dev_config $(OBJECTS)
	$(LD) -r -o lib-loader.o $(OBJECTS) ./linux/lib-os-loader.o ./dev_config/lib-dev-config.o

%.o: %.S Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<

.PHONY: linux
linux:
	@$(MAKE) -C ./linux all

.PHONY: dev_config
dev_config:
	@$(MAKE) -C ./dev_config all

.PHONY: clean
clean:
	cd linux && $(MAKE) -w clean
	cd dev_config && $(MAKE) -w clean
	@$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: