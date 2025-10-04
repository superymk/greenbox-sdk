ifeq ($(TARGET_PLATFORM), i386)
	PAL_LIBC_HEADER = $(SAP_LIBS_BASEDIR)/newlib/dist/i686-secos/include
else ifeq ($(TARGET_PLATFORM), amd64)
	PAL_LIBC_HEADER = $(SAP_LIBS_BASEDIR)/newlib/dist/x86_64-secos/include
endif


INCLUDE_PATH = -I$(PAL_LIBC_HEADER) -I$(PAL_LIBC_HEADERS) -I$(PAL_HEADERS) -I$(SECBASE_PUBLIC_HEADERS) -I$(SECOS_PUBLIC_HEADERS) -I$(APP_HEADERS)/secapp

ifeq ($(TARGET_PLATFORM), i386)
	CFLAGS += -D__X86__ -D__i386__
else ifeq ($(TARGET_PLATFORM), amd64)
	CFLAGS += -D__X86__ -D__amd64__
endif

CFLAGS += $(SELFCONTAIN_FLAGS) -Wall -Werror

CFLAGS_NO_FLOATPOINT = $(CFLAGS) $(SELFCONTAIN_FLAGS_NO_FLOATPOINT)

export INCLUDE_PATH
export CFLAGS_NO_FLOATPOINT

.PHONY: all
all: SAP-LIBC LIBTASK LIBDRV-INIT LIBDRV-USB2 LIBDRV-HID LIBDRV-FB LIB-HID-BRIDGE LIB-TPM-BRIDGE LIB-DTM secapp.lds
	$(LD) -r -o $(BUILD_PATH)/pal-libs.o $(LIBDRV_INIT_BASEDIR)/sap-libdrv-init.o $(LIBDRV_USB2_BASEDIR)/sap-libdrv-usb2.o $(LIBDRV_FB_BASEDIR)/sap-libdrv-fb.o $(LIBDRV_HID_BASEDIR)/sap-libdrv-hid.o $(LIBGUI_BASEDIR)/sap-libgui.o $(LIBBRIDGE_HID_BASEDIR)/sp-libbridge-hid.o $(LIBBRIDGE_TPM_BASEDIR)/sp-libbridge-tpm.o $(LIBDTM_BASEDIR)/sap-libdtm.o ./sap-libc/sap-libc.o

.PHONY: first_build
first_build: newlib_1stbuild LIB-GUI ports
#first_build: LIB-GUI
#first_build: newlib_1stbuild

.PHONY: first_build_clean
first_build_clean: newlib-clean LIB-GUI-clean ports_clean
#first_build_clean: LIB-GUI-clean
#first_build_clean: newlib-clean

.PHONY: secapp.lds
secapp.lds: secapp.lds.S
	@$(ECHO) "Building SecApp LDS"
	gcc -E -x c $(CFLAGS_NO_FLOATPOINT) $< | grep -v '^#' > $@

.PHONY: SAP-LIBC
SAP-LIBC:
	@$(MAKE) -C ./sap-libc
	
.PHONY: LIBTASK
LIBTASK:
	@$(MAKE) -C ./libtask

.PHONY: LIBDRV-INIT
LIBDRV-INIT:
	@$(MAKE) -C $(LIBDRV_INIT_BASEDIR)
	
.PHONY: LIBDRV-USB2
LIBDRV-USB2:
	@$(MAKE) -C $(LIBDRV_USB2_BASEDIR)

.PHONY: LIB-HID-BRIDGE
LIB-HID-BRIDGE:
	@$(MAKE) -C $(LIBBRIDGE_HID_BASEDIR)

.PHONY: LIB-TPM-BRIDGE
LIB-TPM-BRIDGE:
	@$(MAKE) -C $(LIBBRIDGE_TPM_BASEDIR)

.PHONY: LIBDRV-HID
LIBDRV-HID:
	@$(MAKE) -C $(LIBDRV_HID_BASEDIR)

.PHONY: LIBDRV-FB
LIBDRV-FB:
	@$(MAKE) -C $(LIBDRV_FB_BASEDIR)
	
.PHONY: LIB-GUI
LIB-GUI:
	@$(MAKE) -j$(nproc) -C $(LIBGUI_BASEDIR) all

.PHONY: LIB-GUI-clean
LIB-GUI-clean:
	cd $(LIBGUI_BASEDIR) && $(MAKE) -w clean

.PHONY: LIB-DTM
LIB-DTM:
	@$(MAKE) -C $(LIBDTM_BASEDIR)
	
.PHONY: subsys_openbsd
subsys_openbsd:
	@$(MAKE) -C ./subsys_openbsd all
	
.PHONY: newlib_1stbuild
newlib_1stbuild: 
	@$(MAKE) -C ./newlib build_1st

.PHONY: newlib_fastbuild
newlib_fastbuild: 
	@$(MAKE) -C ./newlib build_fast
	
.PHONY: newlib-clean
newlib-clean:
	cd newlib && $(MAKE) -w clean_newlib

.PHONY: ports
ports:
	@$(MAKE) -C ./ports all

.PHONY: ports_clean
ports_clean:
	cd ports && $(MAKE) -w clean

.PHONY: clean
clean:
	cd sap-libc && $(MAKE) -w clean
	cd subsys_openbsd && $(MAKE) -w clean
	cd libtask && $(MAKE) -w clean
	cd $(LIBDRV_INIT_BASEDIR) && $(MAKE) -w clean
	cd $(LIBDRV_USB2_BASEDIR) && $(MAKE) -w clean
	cd $(LIBDRV_FB_BASEDIR) && $(MAKE) -w clean
	cd $(LIBDRV_HID_BASEDIR) && $(MAKE) -w clean
	cd $(LIBBRIDGE_HID_BASEDIR) && $(MAKE) -w clean
	cd $(LIBBRIDGE_TPM_BASEDIR) && $(MAKE) -w clean
	cd $(LIBDTM_BASEDIR) && $(MAKE) -w clean
	@$(RM) *.o