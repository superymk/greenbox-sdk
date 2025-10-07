# Ported libraries are for SecApps (can use float point functions), not for SecServices (cannot use float point functions). 
# [NOTE] Some SecServices (e.g., DesktopMgr) may link newlib, but they cannot libraries in this folder as they may use
# float point functions.


# For all ported libraries
CFLAGS := $(filter-out -Werror,$(CFLAGS))

ifeq ($(TARGET_PLATFORM), i386)
	SECOS_ARCH_STR = i686-secos
else ifeq ($(TARGET_PLATFORM), amd64)
	SECOS_ARCH_STR = x86_64-secos
endif

SAP_PORT_LIBS_CFLAGS += $(INCLUDE_PATH) $(CFLAGS)

SAP_PORT_LIBS_LDFLAGS = -L$(SAP_NEWLIB_DIST)/x86_64-secos/lib -L$(SAP_PORT_LIBS_BASEDIR) -lc -lg -lm -lnosys -lempty

## Newlib
NEWLIB_LIB_DIST_PATH = $(SAP_LIBS_BASEDIR)/newlib/dist/$(SECOS_ARCH_STR)/lib

# Build dirs of libs
BUILD_DIR_OPENSSL = $(SAP_PORT_LIBS_BASEDIR)/openssl/build
BUILD_DIR_LIBSSH2 = $(SAP_PORT_LIBS_BASEDIR)/libssh2/build
BUILD_DIR_ZLIB = $(SAP_PORT_LIBS_BASEDIR)/zlib/build
BUILD_DIR_LIBCURL = $(SAP_PORT_LIBS_BASEDIR)/curl/build
BUILD_DIR_JANSSON = $(SAP_PORT_LIBS_BASEDIR)/jansson/build

# For libssh2
LIBSSH2_CFLAGS = $(SAP_PORT_LIBS_CFLAGS) -DLIBSSH2_SYS_GREENBOX


export SAP_PORT_LIBS_CFLAGS
export SAP_PORT_LIBS_LDFLAGS
export SAP_PORT_LIBS_DIST_DIR

.PHONY: all
all: create_dist_dir lib_tier0 lib_tier1 lib_tier2
#all: lib_empty libcurl
#all: liboauth2 

# Independent libs
.PHONY: lib_tier0
lib_tier0: lib_empty zlib 

# Libs depend on tier0 libs
.PHONY: lib_tier1
lib_tier1: openssl libssh2

# Libs depend on tier1 libs
.PHONY: lib_tier2
lib_tier2: libcurl jansson cjose liboauth2

.PHONY: create_dist_dir
create_dist_dir:
	mkdir -p $(SAP_PORT_LIBS_DIST_DIR)

.PHONY: lib_empty
lib_empty:	
	$(CC) $(SAP_PORT_LIBS_CFLAGS) -c libempty.c -o libempty.o
	$(AR) -rcs libempty.a libempty.o
	
.PHONY: zlib
zlib:
	PREFIX=$(SAP_PORT_LIBS_DIST_DIR) CFLAGS="$(SAP_PORT_LIBS_CFLAGS)" ./build_zlib.sh


.PHONY: openssl
openssl:	
	PREFIX=$(SAP_PORT_LIBS_DIST_DIR) CFLAGS="$(SAP_PORT_LIBS_CFLAGS)" ./build_openssl.sh

.PHONY: libssh2
libssh2:	
	PREFIX=$(SAP_PORT_LIBS_DIST_DIR) CFLAGS="$(LIBSSH2_CFLAGS)" ./build_libssh2.sh

.PHONY: libcurl
libcurl:	
	PREFIX=$(SAP_PORT_LIBS_DIST_DIR) CFLAGS="$(SAP_PORT_LIBS_CFLAGS)" LDFLAGS="$(SAP_PORT_LIBS_LDFLAGS)" ./build_libcurl.sh

.PHONY: jansson
jansson:	
	@$(MAKE) -C ./jansson all
	@$(MAKE) -C ./jansson install
	
.PHONY: cjose
cjose:	
	@$(MAKE) -C ./cjose all
	@$(MAKE) -C ./cjose install
	
.PHONY: liboauth2
liboauth2:	
	PREFIX=$(SAP_PORT_LIBS_DIST_DIR) CFLAGS="$(SAP_PORT_LIBS_CFLAGS) -I./include" LDFLAGS="$(SAP_PORT_LIBS_LDFLAGS)" ./build_liboauth2.sh

.PHONY: clean
clean:
	@$(RMDIR) $(BUILD_DIR_ZLIB)
	@$(RMDIR) $(BUILD_DIR_OPENSSL)
	@$(RMDIR) $(BUILD_DIR_LIBSSH2)
	@$(RMDIR) $(BUILD_DIR_LIBCURL)
	cd jansson && $(MAKE) -w clean
	cd jansson && $(MAKE) -w clean
	@$(RMDIR) $(SAP_PORT_LIBS_DIST_DIR)
	@$(RM) libempty.o libempty.a
	@$(RM) *.o