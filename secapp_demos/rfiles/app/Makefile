# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

current_dir = $(shell pwd)
APP_INCLUDE += -I$(current_dir)/../include

# [TODO] Temporily limit the check on Windows only.
ifeq ($(OS), windows_msys2)
	# If the RFiles QT project has multiple build configurations, they all should output to the Debug folder or 
	# the Release folder.
	ifeq ($(CONFIG_SOSLOADER_GUI_DEBUG), y)
	QT_APP_DIRS := $(current_dir)/ui/RFiles/build/Debug
	else ifeq ($(CONFIG_SOSLOADER_GUI_DEBUG), n)
	QT_APP_DIRS := $(current_dir)/ui/RFiles/build/Release
	endif
endif
 
.PHONY: all
# all: console ui FORCE
all: console FORCE
	
.PHONY: console
console: 
	@$(MAKE) -C ./console all

.PHONY: ui
ui: 
ifeq ($(OS), windows_msys2)
	@$(CP) -rf $(QT_APP_DIRS)/RFiles.exe $(DIST_SAP_PATH)/
	@$(CP) -rf $(QT_APP_DIRS)/RFiles.exe $(DIST_PRIV_SAP_PATH)/
else
	# [TODO] Temporily limit the check on Windows only.
	# @$(CP) -rf $(QT_APP_DIRS)/RFiles $(DIST_SAP_PATH)/
	# @$(CP) -rf $(QT_APP_DIRS)/RFiles $(DIST_PRIV_SAP_PATH)/
endif
	
clean:
	cd console && $(MAKE) -w clean
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: