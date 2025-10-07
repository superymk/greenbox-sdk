# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

APP_INCLUDE += -I../include

.PHONY: all
all: $(OBJECTS) copy_scripts
	
%.o: %.S Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<

.PHONY: copy_scripts
copy_scripts:
	@$(CP) -f ./*.bat $(DIST_SOS_PATH)/
	@$(CP) -f ./*.bat $(DIST_PRIV_SOS_PATH)/

.PHONY: clean
clean:
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: