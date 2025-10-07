# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

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