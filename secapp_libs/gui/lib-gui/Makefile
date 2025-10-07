# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

#
# Makefile
#
CC ?= gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare
CFLAGS += -Wno-unused-variable -Wno-unused-function

CFLAGS := $(filter-out -Werror,$(CFLAGS))

current_dir = $(shell pwd)
INCLUDE_PATH += -I$(current_dir)/include -I$(LIBDRV_HID_PUBLIC_HEADERS)


# Collect the files to compile
include $(LVGL_DIR)/lvgl/lvgl.mk

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)

C_SOURCES += $(CSRCS)
AS_SOURCES += $(ASRCS)

OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

.PHONY: all
all: copy_lvgl_headers $(OBJECTS)
	@$(ECHO) "Building Lib-GUI"  
	$(LD) -r -o sap-libgui.o $(OBJECTS)
	
%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
	@echo "CC $<"
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<
	@echo "CC $<"
	
.PHONY: copy_lvgl_headers
copy_lvgl_headers:
	find ./lvgl -name "*.h" -print0 | cpio -pvd0 $(PAL_HEADERS)/gui/ui/
	find ./ -name "lv_conf.h" -print0 | cpio -pvd0 $(PAL_HEADERS)/gui/ui/


clean: 
	@$(RM) $(OBJECTS)
	@$(RM) sap-libgui.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: