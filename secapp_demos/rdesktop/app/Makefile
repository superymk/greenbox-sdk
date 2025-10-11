# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

current_dir = $(shell pwd)
APP_INCLUDE += -I$(current_dir)/../include

# If the RDesktop QT project has multiple build configurations, they all should output to the Debug folder or 
# the Release folder.
ifeq ($(CONFIG_SOSLOADER_GUI_DEBUG), y)
QT_APP_DIRS := $(current_dir)/ui/RDesktop/build/Debug
else ifeq ($(CONFIG_SOSLOADER_GUI_DEBUG), n)
QT_APP_DIRS := $(current_dir)/ui/RDesktop/build/Release
endif

# Debug Options
ifeq ($(DEBUG),y)
	# Debug - Link Address Sanitizer if <CONFIG_DEBUG_LOADERS_MEMLEAK> is set.
	ifeq ($(CONFIG_DEBUG_LOADERS_MEMLEAK), y)
		CFLAGS += -fsanitize=address
		APP_LFLAGS += -fsanitize=address
	endif
endif
 
.PHONY: all
all: console ui FORCE
	
.PHONY: console
console: 
	@$(MAKE) -C ./console all

.PHONY: ui
ui: 
ifeq ($(OS), windows_msys2)
	@$(CP) -rf $(QT_APP_DIRS)/RDesktop.exe $(DIST_SAP_PATH)/
	@$(CP) -rf $(QT_APP_DIRS)/RDesktop.exe $(DIST_PRIV_SAP_PATH)/
else
	@$(CP) -rf $(QT_APP_DIRS)/RDesktop $(DIST_SAP_PATH)/
	@$(CP) -rf $(QT_APP_DIRS)/RDesktop $(DIST_PRIV_SAP_PATH)/
endif
	
clean:
	cd console && $(MAKE) -w clean
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: