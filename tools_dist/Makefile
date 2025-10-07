# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

export DIST_TOOLS_CONTROL_MONITOR_PATH = $(DIST_TOOLS_PATH)/control_monitor
export DIST_PRIV_TOOLS_CONTROL_MONITOR_PATH = $(DIST_PRIV_TOOLS_PATH)/control_monitor

export DIST_TOOLS_WIN_CURSOR_OVERLAY_PATH = $(DIST_TOOLS_PATH)/win_cursor_overlay
export DIST_PRIV_TOOLS_WIN_CURSOR_OVERLAY_PATH = $(DIST_PRIV_TOOLS_PATH)/win_cursor_overlay

current_dir := $(shell pwd)

# [TODO] Temporily limit the check on Windows only.
ifeq ($(OS), windows_msys2)
	# If the control_monitor QT project has multiple build configurations, they all should output to the Debug folder or 
	# the Release folder.
	ifeq ($(CONFIG_SOSLOADER_GUI_DEBUG), y)
	CONTROL_MONITOR_BUILD_OUT_DIR := $(current_dir)/control_monitor/build/Debug
	else ifeq ($(CONFIG_SOSLOADER_GUI_DEBUG), n)
	CONTROL_MONITOR_BUILD_OUT_DIR := $(current_dir)/control_monitor/build/Release
	endif

	WIN_CURSOR_OVERLAY_BUILD_OUT_DIR := $(current_dir)/win_cursor_overlay
endif


# Makefile options
.PHONY: all
ifeq ($(OS), windows_msys2)
# For Windows, we need to build the win_cursor_overlay too in order to show a mouse cursor after removing all mouses 
# from Windows.
all: tpm
else
all: tpm
endif

.PHONY: tpm
tpm:
	@$(ECHO) "Building the TPM tool..."
	@$(MAKE) -C ./tpm
	
clean:
	cd tpm && $(MAKE) -w clean
