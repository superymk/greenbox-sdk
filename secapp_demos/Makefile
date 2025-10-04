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

ifeq ($(TARGET_PLATFORM), i386)
	SECOS_ARCH_STR = i686-secos
else ifeq ($(TARGET_PLATFORM), amd64)
	SECOS_ARCH_STR = x86_64-secos
endif

PAL_LIBC_HEADER = $(SAP_LIBS_BASEDIR)/newlib/dist/$(SECOS_ARCH_STR)/include
PAL_LIBC_LIB = $(SAP_LIBS_BASEDIR)/newlib/dist/$(SECOS_ARCH_STR)/lib/libc.a $(SAP_LIBS_BASEDIR)/newlib/dist/$(SECOS_ARCH_STR)/lib/libg.a $(SAP_LIBS_BASEDIR)/newlib/dist/$(SECOS_ARCH_STR)/lib/libm.a $(SAP_LIBS_BASEDIR)/newlib/dist/$(SECOS_ARCH_STR)/lib/libnosys.a

# Apps related headers
APP_INCLUDE = -I$(SECOS_PUBLIC_HEADERS) -I/usr/local/include -I$(SECAGENT_PUBLIC_HEADERS) -I$(SECOS_LOADER_PUBLIC_HEADERS) 
APP_INCLUDE += -I$(LOADERS_EXTERNAL_BASEDIR) -I$(APP_HEADERS)

# SecApps related headers
PAL_INCLUDE = -I$(SECBASE_PUBLIC_HEADERS) -I$(SECOS_PUBLIC_HEADERS) -I$(PAL_LIBC_HEADER) -I$(PAL_LIBC_HEADERS) -I$(PAL_HEADERS) -I$(APP_HEADERS)/secapp
PAL_INCLUDE += -I$(SECSRV_INIT_PUBLIC_HEADERS) -I$(IOSM_USB3_PUBLIC_HEADERS)

# SecApps' CFLAGS
PAL_CFLAGS = $(SELFCONTAIN_FLAGS)
## Additonal CFLAGS for SecApps.
PAL_CFLAGS += -fno-omit-frame-pointer

SP_CFLAGS_NO_FLOATPOINT = $(PAL_CFLAGS) $(SELFCONTAIN_FLAGS_NO_FLOATPOINT)


ifeq ($(TARGET_PLATFORM), i386)
    PAL_CFLAGS += -D__X86__ -D__i386__
else ifeq ($(TARGET_PLATFORM), amd64)
	PAL_CFLAGS += -D__X86__ -D__amd64__
endif

# Ensure the stacks are non-executable.
PAL_LFLAGS = -z noexecstack

export APP_INCLUDE
export PAL_INCLUDE
export PAL_CFLAGS
export PAL_LFLAGS
export SP_CFLAGS_NO_FLOATPOINT
export PAL_LIBC_LIB

# If compiling with Cygwin, then we output SecOS loaders and SecApp loaders only. So we do not need to compile SecApps and SecServices
.PHONY: all
all: $(PAL_DEMO)

.PHONY: first_build_clean
first_build_clean:
ifeq ($(PAL_DEMO), RDESKTOP_PAL)
	@$(MAKE) -C rdesktop first_build_clean
endif

.PHONY: first_build
first_build:
ifeq ($(PAL_DEMO), RDESKTOP_PAL)
	@$(MAKE) -C rdesktop first_build
endif
	
.PHONY: RICH_GUI_SAP
RICH_GUI_SAP:
ifeq ($(OS), windows_msys2)
	@$(MAKE) -C ./demo_rich_gui clean
	@$(MAKE) -C ./demo_rich_gui app
else
	@$(MAKE) -C ./demo_rich_gui all
endif

.PHONY: RDESKTOP_PAL
RDESKTOP_PAL:
ifeq ($(OS), windows_msys2)
	@$(MAKE) -C ./rdesktop clean
	@$(MAKE) -C ./rdesktop app
else
	@$(MAKE) -C ./rdesktop all
endif

.PHONY: RFILES_SAP
RFILES_SAP:
ifeq ($(OS), windows_msys2)
	@$(MAKE) -C ./rfiles clean
	@$(MAKE) -C ./rfiles app
else
	@$(MAKE) -C ./rfiles all
endif

.PHONY: SP_EXAMPLE_SAP
SP_EXAMPLE_SAP:
ifeq ($(OS), windows_msys2)
	@$(MAKE) -C ./sp_example clean
	@$(MAKE) -C ./sp_example app
else
	@$(MAKE) -C ./sp_example all
endif
	
.PHONY: SP_TEST_PAL
SP_TEST_PAL:
ifeq ($(OS), windows_msys2)
	@$(MAKE) -C ./sp_test clean
	@$(MAKE) -C ./sp_test app
else
	@$(MAKE) -C ./sp_test all
endif

.PHONY: clean
clean:
	cd demo_rich_gui && $(MAKE) -w clean
	cd rdesktop && $(MAKE) -w clean
	cd rfiles && $(MAKE) -w clean
	cd sp_example && $(MAKE) -w clean
	cd sp_test && $(MAKE) -w clean