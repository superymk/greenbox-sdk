# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

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