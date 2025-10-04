ifeq ($(TARGET_PLATFORM), i386)
    export SAP_AR = $(SAP_NEWLIB_DIST)/bin/i686-secos-ar
    export SAP_AS = $(SAP_NEWLIB_DIST)/bin/i686-secos-as
    export SAP_CC = $(SAP_NEWLIB_DIST)/bin/i686-secos-gcc
    #export SAP_CPP = $(SAP_NEWLIB_DIST)/bin/i686-secos-g++
    export SAP_LD = $(SAP_NEWLIB_DIST)/bin/i686-secos-ld
    export SAP_OBJCOPY = $(SAP_NEWLIB_DIST)/bin/i686-secos-objcopy
    export SAP_RANLIB = $(SAP_NEWLIB_DIST)/bin/i686-secos-ranlib
    export SAP_STRIP = $(SAP_NEWLIB_DIST)/bin/i686-secos-strip
else ifeq ($(TARGET_PLATFORM), amd64)
    export SAP_AR = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-ar
    export SAP_AS = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-as
    export SAP_CC = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-gcc
    #export SAP_CPP = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-g++
    export SAP_LD = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-ld
    export SAP_OBJCOPY = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-objcopy
    export SAP_RANLIB = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-ranlib
    export SAP_STRIP = $(SAP_NEWLIB_DIST)/bin/x86_64-secos-strip
endif