# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

CPP_SOURCES = $(wildcard ../gui/*.cpp)
AS_SOURCES = $(wildcard ../gui/*.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.cpp, %.o, $(CPP_SOURCES))

APP_INCLUDE += -I$(SECOS_LOADER_BASEDIR)/include -I$(SECOS_LOADER_BASEDIR)/include/sl_server -I$(LOADERS_EXTERNAL_BASEDIR) -I$(APP_HEADERS)
APP_INCLUDE += -I../cJSON
APP_INCLUDE += -I../hash
APP_INCLUDE += -I../loader/include

# ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
# 	WX_CPP_FLAGS := $(shell echo `/mingw64/bin/wx-config --cxxflags`)
# 	WX_LIBS_FLAGS := $(shell echo `/mingw64/bin/wx-config --libs`)
# 	CFLAGS := -O3 $(filter-out -g -O%, $(CFLAGS))
# else 
ifeq ($(CONFIG_OS_LINUX), y)
	WX_CPP_FLAGS := $(shell echo `wx-config --cxxflags`)
	WX_LIBS_FLAGS := $(shell echo `wx-config --libs`)
endif

# ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
# 	ifeq ($(TARGET_PLATFORM), i386)
# 		APP_INCLUDE += -I/mingw32/lib/wx/include/msw-unicode-3.0/ -I/mingw32/include/wx-3.0
# 	else ifeq ($(TARGET_PLATFORM), amd64)
# 		APP_INCLUDE += -I/mingw64/lib/wx/include/msw-unicode-3.0/ -I/mingw64/include/wx-3.0
# 	endif
# endif

# Remove "-Werror" because 3rd party libraries may have compile warnings
CFLAGS := $(filter-out -Werror,$(CFLAGS))

.PHONY: all 
all: c-json hash loader $(OBJECTS) copy_res
ifeq ($(CONFIG_OS_LINUX), y)
	$(LD) -r -o ../lib-gui.o $(OBJECTS) ../loader/lib-loader.o ../cJSON/*.o ../hash/lib-hash.o 
endif
	
.PHONY: copy_res
copy_res: $(OBJECTS)
	@$(CP) -rf ../gui/gui_res $(DIST_SOSLOADER_GUI_PATH)/gui_res
	@$(CP) -rf ../gui/gui_res $(DIST_PRIV_SOSLOADER_GUI_PATH)/gui_res

.PHONY: c-json
c-json:
	@$(MAKE) -C ../cJSON static

.PHONY: hash
hash:
	@$(MAKE) -C ../hash all

.PHONY: loader
loader:
	@$(MAKE) -C ../loader all
	
%.o: %.S Makefile
	$(WXW_CPP) -c $(APP_INCLUDE) $(CFLAGS) $(WX_CPP_FLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.cpp Makefile
	$(WXW_CPP) -c $(APP_INCLUDE) $(CFLAGS) $(WX_CPP_FLAGS) -o $@ $<

.PHONY: clean
clean:
	cd ../hash && $(MAKE) -w clean
	cd ../loader && $(MAKE) -w clean
	$(RM) ../gui/*.o