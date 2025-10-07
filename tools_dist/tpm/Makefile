# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

export DIST_TOOLS_TPM_PATH = $(DIST_TOOLS_PATH)/tpmtools
export DIST_PRIV_TOOLS_TPM_PATH = $(DIST_PRIV_TOOLS_PATH)/tpmtools

C_SOURCES = $(wildcard *.c)
AS_SOURCES = $(wildcard *.S)

C_SOURCES += $(wildcard ./hash/*.c)
AS_SOURCES += $(wildcard ./hash/*.S)

C_SOURCES += $(wildcard ./efi_event_log/*.c)
AS_SOURCES += $(wildcard ./efi_event_log/*.S)

OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(C_SOURCES))

current_dir := $(shell pwd)

INCLUDE_PATH = -I$(SECOS_PUBLIC_HEADERS) -I$(SECAGENT_PUBLIC_HEADERS) -I$(SECOS_LOADER_PUBLIC_HEADERS) -I$(APP_HEADERS) -I$(SECBASE_PUBLIC_HEADERS)
INCLUDE_PATH += -I$(current_dir)/include

ifeq ($(OS), windows_msys2)
INCLUDE_PATH += -I$(current_dir)/wolfTPM/win/include
else
INCLUDE_PATH += -I$(current_dir)/wolfTPM/linux/include
endif

# Embed SHA256 of the TPM tools' sub tools <tpm/sub_tools> into the caller of the executable. This is for Windows only
# because TPM tools replaces sub tools with bash scripts on Linux.
## Path to the sub tools executable
ifeq ($(OS), windows_msys2)
	GB_TPM_SUBTOOLS_EXECUTABLE := $(current_dir)/sub_tools/efi_tpm2_event_logs.exe
endif

## Function to compute and format SHA256 checksum
define GB_TPM_SUBTOOLS_EXECUTABLE_COMPUTE_SHA256
	sha256sum $(GB_TPM_SUBTOOLS_EXECUTABLE) | \
	sed -e 's/ .*//; s/\([0-9a-f][0-9a-f]\)/0x\1, /g; s/, $$//'
endef

## Add the formatted SHA256 checksum as a -D option
ifeq ($(OS), windows_msys2)
	## Compute the SHA256 checksum and format it
	GB_TPM_SUBTOOLS_EXECUTABLE_SHA256 := $(shell $(GB_TPM_SUBTOOLS_EXECUTABLE_COMPUTE_SHA256))

	## Add the formatted SHA256 checksum as a -D option
	CFLAGS += -DGB_TPM_SUBTOOLS_EXECUTABLE_SHA256="$(GB_TPM_SUBTOOLS_EXECUTABLE_SHA256)"
endif


.PHONY: all
all: copy_lib_tools copy_lib_tools_priv copy_sub_tools copy_sub_tools_priv c-json $(OBJECTS)
ifeq ($(OS), windows_msys2)
	$(CC) -o $(DIST_TOOLS_TPM_PATH)/gb_tpm $(OBJECTS) ./cJSON/*.o -L$(DIST_TOOLS_TPM_PATH) -l:libwolftpm-16.dll -l:libwolfssl-42.dll
	$(STRIP) $(DIST_TOOLS_TPM_PATH)/gb_tpm.exe
	$(CC) -o $(DIST_PRIV_TOOLS_TPM_PATH)/gb_tpm $(OBJECTS) ./cJSON/*.o -L$(DIST_TOOLS_TPM_PATH) -l:libwolftpm-16.dll -l:libwolfssl-42.dll
else
	$(CC) -o $(DIST_TOOLS_TPM_PATH)/gb_tpm $(OBJECTS) ./cJSON/*.o -L$(DIST_TOOLS_TPM_PATH) -Wl,-rpath,. -lwolftpm -lwolfssl
	$(STRIP) $(DIST_TOOLS_TPM_PATH)/gb_tpm
	$(CC) -o $(DIST_PRIV_TOOLS_TPM_PATH)/gb_tpm $(OBJECTS) ./cJSON/*.o -L$(DIST_TOOLS_TPM_PATH) -Wl,-rpath,. -lwolftpm -lwolfssl
endif

%.o: %.S Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.c Makefile FORCE
	$(CC) -c $(INCLUDE_PATH) $(CFLAGS) -o $@ $<

.PHONY: mkdir_lib_tools
mkdir_lib_tools:
	$(MKDIR) $(DIST_TOOLS_TPM_PATH)

.PHONY: copy_lib_tools
copy_lib_tools: mkdir_lib_tools
ifeq ($(OS), windows_msys2)
	@$(CP) -f $(current_dir)/wolfTPM/win/$(TARGET_PLATFORM)/* $(DIST_TOOLS_TPM_PATH)/
	$(MKDIR) $(DIST_TOOLS_TPM_PATH)/openssl/
	$(CP) -f $(current_dir)/openssl/* $(DIST_TOOLS_TPM_PATH)/openssl/
	@$(CP) -f $(current_dir)/*.bat $(DIST_TOOLS_TPM_PATH)/
else
	@$(CP) -f $(current_dir)/wolfTPM/linux/$(TARGET_PLATFORM)/* $(DIST_TOOLS_TPM_PATH)/
	# Change rpath of libwolftpm.so and libwolftpm.so.* so they can find the libwolfssl.so in the current directory.
	patchelf --set-rpath . $(DIST_TOOLS_TPM_PATH)/libwolftpm.so
	patchelf --set-rpath . $(DIST_TOOLS_TPM_PATH)/libwolftpm.so.*
	@$(CP) -f $(current_dir)/*.sh $(DIST_TOOLS_TPM_PATH)/
endif

# Generate tools with symbol to $(DIST_PRIV_TOOLS_TPM_PATH)
.PHONY: copy_lib_tools_priv
copy_lib_tools_priv: mkdir_lib_tools_priv
ifeq ($(OS), windows_msys2)
	@$(CP) -f $(current_dir)/wolfTPM/win/$(TARGET_PLATFORM)/* $(DIST_PRIV_TOOLS_TPM_PATH)/
else
	@$(CP) -f $(current_dir)/wolfTPM/linux/$(TARGET_PLATFORM)/* $(DIST_PRIV_TOOLS_TPM_PATH)/
	@$(CP) -f $(current_dir)/*.sh $(DIST_PRIV_TOOLS_TPM_PATH)/
endif

.PHONY: copy_sub_tools
copy_sub_tools: mkdir_lib_tools
ifeq ($(OS), windows_msys2)
	@$(CP) -f $(current_dir)/sub_tools/*.exe $(DIST_TOOLS_TPM_PATH)/
endif

.PHONY: copy_sub_tools_priv
copy_sub_tools_priv: mkdir_lib_tools_priv
ifeq ($(OS), windows_msys2)
	@$(CP) -f $(current_dir)/sub_tools/*.exe $(DIST_PRIV_TOOLS_TPM_PATH)/
endif

# Generate tools with symbol to $(DIST_PRIV_TOOLS_TPM_PATH)
.PHONY: mkdir_lib_tools_priv
mkdir_lib_tools_priv:
	$(MKDIR) $(DIST_PRIV_TOOLS_TPM_PATH)

.PHONY: c-json
c-json:
	@$(MAKE) -C ./cJSON static


	
.PHONY: clean
clean:
	cd cJSON && $(MAKE) -w clean
	@$(RM) ./hash/*.o
	@$(RM) ./efi_event_log/*.o
	@$(RM) *.o gb_tpm

# Define a phony target that always runs
.PHONY: FORCE
FORCE: