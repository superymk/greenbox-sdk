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

INCLUDE_PATH += -I$(SECOS_SRVS_BASEDIR)/subsys_openbsd/include

CFLAGS += -D_KERNEL

.PHONY: all
all: expand_arch_include sys $(OBJECTS)
	@$(ECHO) "Building SDDK Subsystem - OpenBSD..."
	@$(LD) $(SD_LFLAGS) -r -o $(BUILD_PATH)/libsubsys_openbsd.o ./sys/arch.o

%.o: %.S Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: sys
sys:
	@$(MAKE) -C ./sys all

.PHONY: expand_arch_include
expand_arch_include:
	@$(RMDIR) ./include/machine
	@$(CP) -rf ./include/arch/$(TARGET_PLATFORM)/ ./include/machine/

.PHONY: clean
clean:
	cd sys && $(MAKE) -w clean
	@$(RMDIR) ./include/machine
	@$(RM) *.o