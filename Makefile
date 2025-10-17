# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

include ./config.mk

GCCVERSIONGTEQ9 := $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \>= 9)

ifeq ($(OS), linux)
export CONFIG_OS_LINUX = y
else ifeq ($(OS), windows_msys2)
export CONFIG_OS_WINDOWS_MSYS2 = y
endif

ifeq ($(DEBUG), y)
export DEBUG_FLAGS = -g -DDEBUG -ggdb -O0
else ifeq ($(DEBUG), n)
export DEBUG_FLAGS = -O2 -DNDEBUG
endif

CFLAGS += -DOS_SECOS

## Use the legacy UXMHF64 v1 or not.
ifeq ($(CONFIG_USE_LEGACY_UXMHF64_V1), y)
CFLAGS += -DGREENBOX_USE_UXMHF64_V1
endif

# We only compile perf module when we need the output
ifeq ($(VERBOSE_LVL), V_DEBUG)
export CFLAGS += -DPERF
else ifeq ($(VERBOSE_LVL), V_PERF)
export CFLAGS += -DPERF 
endif

ifdef XMHF_LOG_BUF_SIZE
CFLAGS += -DXMHF_LOG_BUF_SIZE=$(XMHF_LOG_BUF_SIZE)
endif

CFLAGS += -fno-zero-initialized-in-bss -mno-tls-direct-seg-refs
CFLAGS += -Wall
CFLAGS += -D$(VERBOSE_LVL)
CFLAGS += $(DEBUG_FLAGS)

SELFCONTAIN_FLAGS = -static -fno-omit-frame-pointer 
SELFCONTAIN_FLAGS += -fno-builtin -fno-common -fno-strict-aliasing -iwithprefix include
SELFCONTAIN_FLAGS += -fno-stack-protector
SELFCONTAIN_FLAGS += -Wno-pointer-arith -Wfloat-equal
SELFCONTAIN_FLAGS += -Wno-sign-compare
SELFCONTAIN_FLAGS += -Wno-bad-function-cast -Wall
SELFCONTAIN_FLAGS += -Winline
SELFCONTAIN_FLAGS += -march=k8
SELFCONTAIN_FLAGS += -nostdinc -pipe
SELFCONTAIN_FLAGS += -Wno-address-of-packed-member
SELFCONTAIN_FLAGS += -fno-pie -no-pie -fno-pic -fno-plt
SELFCONTAIN_FLAGS += -mno-red-zone  # Prevent interrupt handlers from corrupting "red zones"
# Disable new instructions sets
SELFCONTAIN_FLAGS += -mno-sse4.2 -mno-avx -mno-aes
SELFCONTAIN_FLAGS += -mno-sse5

# Additional SELFCONTAIN_FLAGS to rule out float point functions
SELFCONTAIN_FLAGS_NO_FLOATPOINT = -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3
SELFCONTAIN_FLAGS_NO_FLOATPOINT += -mno-sse4.1 -mno-sse4.2 -mno-sse4 -mno-avx -mno-aes
SELFCONTAIN_FLAGS_NO_FLOATPOINT += -mno-pclmul -mno-sse4a -mno-3dnow -mno-popcnt -mno-abm
SELFCONTAIN_FLAGS_NO_FLOATPOINT += -mno-sse5

ifeq ($(TARGET_PLATFORM), i386)
	CFLAGS += -m32 -D__i386__
	SELFCONTAIN_FLAGS += -m32 -D__i386_

	# x86 only - Control Flow Protections
	SELFCONTAIN_FLAGS += -fcf-protection=full
else ifeq ($(TARGET_PLATFORM), amd64)
	CFLAGS += -m64 -D__amd64__
	SELFCONTAIN_FLAGS += -m64 -D__amd64__

	# x86 only - Control Flow Protections
	SELFCONTAIN_FLAGS += -fcf-protection=full
endif

ifeq ($(CONFIG_OS_LINUX), y)
	CFLAGS += -D__LINUX__
# else ifeq ($(CONFIG_OS_WINDOWS_CYGWIN), y)
# 	CFLAGS += -L/lib/w32api -I/usr/include/w32api -D__WINDOWS__ -D__CYGWIN__
else ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	CFLAGS += -D__WINDOWS__ -D__MSYS2__
endif

ifeq "$(GCCVERSIONGTEQ9)" "1"      # Do not add "-Werror" if GCC is below version 9 [TODO: We are not sure about the exact number. We only know GCC 4 cannot compile the code with -Werror]
	CFLAGS += -Werror
	SELFCONTAIN_FLAGS += -Werror
endif

# Function: if_dir_exists_bool
# Usage: $(call if_dir_exists_bool, <directory>)
# Returns "true" if the directory exists, otherwise "false".
if_dir_exists_bool = $(shell test -d $(1) && echo true || echo false)

# Embed version number
GREENBOX_VERSION_NUMBER := $(shell echo `date +'%Y%m%d%H%M%S'`)
SELFCONTAIN_FLAGS += -DGREENBOX_VERSION_NUMBER=$(GREENBOX_VERSION_NUMBER)

export PLATFORM
export DEBUG_FLAGS
export CFLAGS
export SELFCONTAIN_FLAGS
export SELFCONTAIN_FLAGS_NO_FLOATPOINT
export GREENBOX_VERSION_NUMBER

# Private variables for recipes
ifeq ($(CONFIG_SECAGENT_WIN_DEBUG), y)
	SECAGENT_DIR = ./os_module/win/output/x64/Debug
else
	SECAGENT_DIR = ./os_module/win/output/x64/Release
endif

ifeq ($(CONFIG_SOSLOADER_GUI), y)
	ifeq ($(CONFIG_SOSLOADER_GUI_DEBUG), y)
		SOSLOADER_DIR = ./loaders/sos_loader_gui_client/win/SOSLoader_GUI/x64/Debug
	else
		SOSLOADER_DIR = ./loaders/sos_loader_gui_client/win/SOSLoader_GUI/x64/Release
	endif
endif

# Loaders must be built after tools in order to know their hashes if necessary.
# SecApps must be built after loaders in order to use the app-libs.o in the loaders.
.PHONY: all
all: PRE_BUILD tools SecApps-lib SecAgent loaders SecApps installer POST_BUILD

.PHONY: first_build
first_build: first_build_clean
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) first_build
	@$(MAKE) -C $(SAP_DEMOS) first_build

.PHONY: first_build_clean
first_build_clean:
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) first_build_clean
	@$(MAKE) -C $(SAP_DEMOS) first_build_clean
	
.PHONY: test
test:
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) ports
	# @$(MAKE) -C $(SAP_LIBS_BASEDIR) first_build

.PHONY: test-clean
test-clean:
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) ports-clean
	
.PHONY: tools_dist
tools_dist:
	@$(ECHO) "Building the GreenBox Dist Tools..."
	@$(MAKE) -C ./tools_dist
	
.PHONY: tools
tools: tools_dist

.PHONY: PRE_BUILD
PRE_BUILD:
	$(MKDIR) $(BUILD_PATH)
	$(MKDIR) $(DIST_PATH)
	$(MKDIR) $(DIST_DATA_PATH)
	$(MKDIR) $(DIST_TOOLS_PATH)
	$(MKDIR) $(DIST_SAP_PATH)
	$(MKDIR) $(DIST_SOS_PATH)
	$(MKDIR) $(DIST_INSTALLER_PATH)
	$(MKDIR) $(DIST_LIBS_QT_PATH)
	$(MKDIR) $(DIST_PRIV_PATH)
	$(MKDIR) $(DIST_PRIV_DATA_PATH)
	$(MKDIR) $(DIST_PRIV_TOOLS_PATH)
	$(MKDIR) $(DIST_PRIV_SAP_PATH)
	$(MKDIR) $(DIST_PRIV_SOS_PATH)
	$(MKDIR) $(DIST_PRIV_INSTALLER_PATH)
	$(MKDIR) $(DIST_PRIV_LIBS_QT_PATH)
	
.PHONY: saplibs-ports-first
saplibs-ports-first:
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) ports-clean
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) ports
	
.PHONY: libc
libc:
	@$(ECHO) "Building the libc for SecApps..."
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) newlib_fastbuild
	
.PHONY: SecAgent
SecAgent:
	@$(ECHO) "Building the OS Modules..."
ifeq ($(OS), linux)
	@$(MAKE) -C $(SECAGENT_PATH)/linux
endif

.PHONY: loaders
loaders: PRE_BUILD
	@$(ECHO) "Building Loaders..."
	@$(MAKE) -C $(LOADERS_BASEDIR) all

.PHONY: SecApps-lib
SecApps-lib: libc
	@$(ECHO) "Building the SecApp Library..."
	@$(MAKE) -C $(SAP_LIBS_BASEDIR) all

.PHONY: SecApps
SecApps:
	@$(ECHO) "Building the Demos..."
	@$(MAKE) -C $(SAP_DEMOS) all
	
# Processing binaries after building them (e.g., sign binaries)
.PHONY: POST_BUILD
POST_BUILD:

# Delete linux GreenBox executables from ./win/imgs. Otherwise, msys2 may add ".exe" postfix when copying them to ../output/dist.
.PHONY: _WIN_POST_BUILD_delete_linux_exe
_WIN_POST_BUILD_delete_linux_exe:
	@$(RM) ./win/imgs/sos/app_srv_dwm
	@$(RM) ./win/imgs/sos/secagent.ko
	@$(RM) ./win/imgs/sos/sos_loader
	@$(RM) ./win/imgs/sap/app
	@$(RM) ./win/imgs/sap/sl_demo_sp
	@$(RM) ./win/imgs/sap/sl_rdesktop
	@$(RM) ./win/imgs/sap/sl_rfiles
	@$(RM) ./win/imgs/tools/debugger

# Distribute binaries (no symbols) to $(DIST_PATH)
.PHONY: WIN_POST_BUILD
WIN_POST_BUILD: _WIN_POST_BUILD_delete_linux_exe WIN_POST_BUILD_PRIV
	@$(CP) -f ./win/command.bat $(DIST_SOS_PATH)/
	@$(CP) -rf ./win/imgs/* $(DIST_PATH)/
	@$(CP) -f ./win/cygwin/*.dll $(DIST_SOS_PATH)/
	@$(CP) -f ./win/libusb/*.dll $(DIST_SOS_PATH)/
	@$(CP) -f ./win/cygwin/*.dll $(DIST_SAP_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.inf $(DIST_SOS_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.cer $(DIST_SOS_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.pdb $(DIST_SOS_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.sys $(DIST_SOS_PATH)/
ifeq ($(CONFIG_SOSLOADER_GUI), y)
	@$(CP) -f $(SOSLOADER_DIR)/SOSLoader_GUI.exe $(DIST_SOSLOADER_GUI_PATH)/
	@$(CP) -f $(SOSLOADER_DIR)/*.dll $(DIST_SOSLOADER_GUI_PATH)/
	@$(CP) -rf ./loaders/sos_loader_gui_client/gui/gui_res $(DIST_SOSLOADER_GUI_PATH)/
endif

# Distribute binaries (with symbols) to $(DIST_PRIV_PATH)
.PHONY: WIN_POST_BUILD_PRIV
WIN_POST_BUILD_PRIV:
	@$(CP) -f ./win/command.bat $(DIST_PRIV_SOS_PATH)/
	@$(CP) -rf ./win/imgs/* $(DIST_PRIV_PATH)/
	@$(CP) -f ./win/cygwin/*.dll $(DIST_PRIV_SOS_PATH)/
	@$(CP) -f ./win/libusb/*.dll $(DIST_PRIV_SOS_PATH)/
	@$(CP) -f ./win/cygwin/*.dll $(DIST_PRIV_SAP_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.inf $(DIST_PRIV_SOS_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.cer $(DIST_PRIV_SOS_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.pdb $(DIST_PRIV_SOS_PATH)/
	@$(CP) -f $(SECAGENT_DIR)/SecAgent.sys $(DIST_PRIV_SOS_PATH)/
ifeq ($(CONFIG_SOSLOADER_GUI), y)
	@$(CP) -f $(SOSLOADER_DIR)/SOSLoader_GUI.exe $(DIST_PRIV_SOSLOADER_GUI_PATH)/
	@$(CP) -f $(SOSLOADER_DIR)/*.dll $(DIST_PRIV_SOSLOADER_GUI_PATH)/
	@$(CP) -rf ./loaders/sos_loader_gui_client/gui/gui_res $(DIST_PRIV_SOSLOADER_GUI_PATH)/
endif

# Loaders must be built after tools in order to know their hashes if necessary.
# SecApps must be built after loaders in order to use the app-libs.o in the loaders.
.PHONY: win
win: PRE_BUILD loaders SecApps installer WIN_POST_BUILD

.PHONY: win_copy_files_only
win_copy_files_only: WIN_POST_BUILD

. PHONY: win_gui
win_gui:
	@$(ECHO) "Building GUI Loaders..."
	@$(MAKE) -C $(LOADERS_BASEDIR) sos_loader_gui_client
	
.PHONY: win_mingw_tools
win_mingw_tools: tools_dist

.PHONY: win_mingw_tools_clean
win_mingw_tools_clean:
	cd tools_dist && $(MAKE) -w clean

.PHONY: doc
doc:
	@$(ECHO) "Building the Documents"
	@$(RMDIR) ./doc
	doxygen doxy_config.txt

.PHONY: installer
installer:
ifeq ($(OS), linux)
	# @$(CP) -rf ./installer/linux/* $(DIST_INSTALLER_PATH)/
	# @$(CP) -rf ./installer/linux/* $(DIST_PRIV_INSTALLER_PATH)/
	@$(ECHO) "Generating installer (Linux) - Unimplemented."
else ifeq ($(OS), windows_msys2)
	# @$(CP) -rf ./installer/win/* $(DIST_INSTALLER_PATH)/
	# @$(CP) -rf ./installer/win/* $(DIST_PRIV_INSTALLER_PATH)/
	@$(ECHO) "Generating installer (Win) - Unimplemented."
endif

.PHONY: public
public:
	#~ strip --strip-debug --strip-unneeded --discard-all --keep-symbol=vk_start_pal ./build/*.o ./secapp 
	strip --strip-debug --strip-unneeded --discard-all ./verifiable-kernel.img
	# Make dir if not exist
	$(MKDIR) ~/whitebox_public/code/examples
	$(MKDIR) ~/whitebox_public/code/os_module
	$(MKDIR) ~/whitebox_public/code/tools
	$(MKDIR) ~/whitebox_public/code/build
	
	# Copy empty makefile
	@$(CP) -rf ./tools_dist/util/Makefile_empty ~/whitebox_public/code/vk/Makefile
	@$(CP) -rf ./tools_dist/util/Makefile_empty ~/whitebox_public/code/os_module/Makefile
	@$(CP) -rf ./tools_dist/util/Makefile_empty ~/whitebox_public/code/tools/Makefile
	# Copy binaries
	@$(CP) -f ./verifiable-kernel.img ~/whitebox_public/code/
	
	@$(CP) -rf ./examples/Makefile ~/whitebox_public/code/examples/
	# Link sub-folders
	ln -sf $(ROOT_DIR)/app_dev ~/whitebox_public/code/
	ln -sf $(ROOT_DIR)/secapp_dev ~/whitebox_public/code/
	ln -sf $(ROOT_DIR)/xmhf-v0.2.2 ~/whitebox_public/code/
	ln -sf $(ROOT_DIR)/examples/dummy ~/whitebox_public/code/examples/
	ln -sf $(ROOT_DIR)/examples/rdesktop ~/whitebox_public/code/examples/
	ln -sf $(ROOT_DIR)/examples/graphics ~/whitebox_public/code/examples/
	# Copy files in the root folder
	@$(CP) -f ./*.sh ~/whitebox_public/code/
	@$(CP) -f ./*.mk ~/whitebox_public/code/
	@$(CP) -f ./*.ld ~/whitebox_public/code/
	@$(CP) -f ./Makefile ~/whitebox_public/code/
	@$(CP) -rf ./os_module/secagent.ko ~/whitebox_public/code/os_module/
	rm -rf ~/whitebox_public/.git
	# Make the tar ball
	tar zcvfh ~/whitebox_public.tar.gz -C ~/ whitebox_public > /dev/null
	
.PHONY: world
world: all doc tools

# Code verification
.PHONY: verify
verify:
	@$(MAKE) -C ./secos verify
	
# For debugging the verification
.PHONY: v_debug
v_debug:
	@$(MAKE) -C ./secos v_debug

.PHONY: clean_dist_priv
clean_dist_priv:
	@$(RM) -rf $(DIST_PRIV_PATH)
	
# Delete executables only
.PHONY: clean_dist_executables
clean_dist_executables:
	@if [ "$(call if_dir_exists_bool, $(DIST_PATH))" = "true" ]; then \
		find $(DIST_PATH) -mindepth 1 -path $(DIST_DATA_PATH) -prune -o -exec rm -rf {} +; \
	else \
	    echo "Directory $(DIST_PATH) does not exist."; \
	fi
	
# Delete all files (i.e., executables and userdata) in dist
.PHONY: clean_dist_all
clean_dist_all:
ifeq ($(OS), linux)
	sudo $(RM) -rf $(DIST_PATH)
else ifeq ($(OS), windows_msys2)
	@$(RM) -rf $(DIST_PATH)
endif
	
.PHONY: win_clean
win_clean: clean_dist_executables
	@$(RM) -rf $(BUILD_PATH)
	cd $(LOADERS_BASEDIR) && $(MAKE) -w clean
	cd $(SAP_DEMOS) && $(MAKE) -w clean
	
	
.PHONY: clean
clean: clean_dist_executables clean_dist_priv
	@$(RM) -rf $(BUILD_PATH)
	cd $(SAP_LIBS_BASEDIR) && $(MAKE) -w clean
	cd $(LOADERS_BASEDIR) && $(MAKE) -w clean
ifeq ($(OS), linux)
	cd $(SECAGENT_PATH)/linux && $(MAKE) -w clean
endif
	cd $(SAP_DEMOS) && $(MAKE) -w clean
	cd tools_dist && $(MAKE) -w clean
	@$(RM) *.o core.* secapp
	
