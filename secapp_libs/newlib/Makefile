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

NEWLIB_BUILD=$(SAP_LIBS_BASEDIR)/newlib/build
NEWLIB_DIST=$(SAP_NEWLIB_DIST)

# machine where SecApps will run
ifeq ($(TARGET_PLATFORM), i386)
	HOST=i686-linux-gnu
	TARGET=i686-secos
else ifeq ($(TARGET_PLATFORM), amd64)
	HOST=x86_64-linux-gnu
	TARGET=x86_64-secos
endif

# machine where software will be built
ifeq ($(CURRENT_PLATFORM), i386)
	BUILD=i686-linux-gnu
else ifeq ($(CURRENT_PLATFORM), amd64)
	BUILD=x86_64-linux-gnu
endif

# # Newlib CFLAGS
# ifeq ($(TARGET_PLATFORM), i386)
# 	NEWLIB_CFLAGS += -U__CYGWIN__ -UWIN32 -m32
# else ifeq ($(TARGET_PLATFORM), amd64)
# 	NEWLIB_CFLAGS += -U__CYGWIN__ -UWIN32 -m64
# endif

# ifeq ($(OS), windows_msys2)
# 	NEWLIB_CFLAGS += -I/usr/include/
# endif


#~ NEWLIB_CFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3
#~ NEWLIB_CFLAGS += -mno-sse4.1 -mno-sse4.2 -mno-sse4 -mno-avx -mno-aes
#~ NEWLIB_CFLAGS += -mno-pclmul -mno-sse4a -mno-3dnow -mno-popcnt -mno-abm
#~ NEWLIB_CFLAGS += -mno-sse5

# where reg-world software will be installed
PREFIX=$(NEWLIB_DIST)
export PATH := $(PREFIX)/bin:$(PATH)

.PHONY: build_1st
build_1st: toolchain
	mkdir -p $(NEWLIB_BUILD)
	mkdir -p $(NEWLIB_DIST)
	PREFIX=$(PREFIX) HOST=$(HOST) TARGET=$(TARGET) CFLAGS="$(NEWLIB_CFLAGS)" ./build_first.sh
	
.PHONY: build_fast
build_fast: 
	@$(MAKE) -C ./build all
	@$(MAKE) -C ./build install
	
.PHONY: toolchain
toolchain:
	cd toolchain && \
	./autogen.sh && \
	./configure --prefix=$(PREFIX) --host=$(HOST) --target=$(TARGET) --build=$(BUILD) && \
	make && \
	make install

.PHONY: clean_newlib
clean_newlib:
	rm -rf ./build ./dist
	